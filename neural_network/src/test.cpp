#include <cmath>
#include <iomanip>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

static double sigmoid(const double &z);

int main() {
	string init_file, test_file, result_file;
	ifstream ifs;
	ofstream ofs;
	char delimiter = ' ';
	int num_input_nodes, num_hidden_nodes, num_output_nodes;
	int num_train_examples, num_input_nodes_test, num_output_nodes_test;
	
	double microavg[4]; // a, b, c, d (TP, FN, FP, TN)
	double macroavg[3]; // accuracy, precision, recall (f1 is computed separately)

	int a,b,c,d; 
	double accuracy, precision, recall, f1;

	while(1) {
		cout << "Enter filename for trained network: ";
		cin >> init_file;
		ifs.open(init_file.c_str());
		// exit if file is opened
		if(ifs.is_open()) {
			break;
		} else {
			cerr << "[Error] Missing file: " << init_file << endl;
		}
	}
	// parse first line which contains the number of input, hidden, output nodes
	// delimited by single space, ' '
	string buffer;
	getline(ifs, buffer, delimiter);
	num_input_nodes = stoi(buffer);

	getline(ifs, buffer, delimiter);
	num_hidden_nodes = stoi(buffer);
	
	getline(ifs, buffer);
	num_output_nodes = stoi(buffer);

	double w_input_hidden[num_hidden_nodes][num_input_nodes + 1];
	double w_hidden_output[num_output_nodes][num_hidden_nodes + 1];
	double w_input_hidden2[num_input_nodes];
	double hidden_a[num_hidden_nodes];
	double output_a[num_output_nodes];
	int output_boolean[num_output_nodes];
	int result_file_statistics[num_output_nodes][4];
	
	/* 1. Network weight initialization from file START */
	// initialize list elements to zeros
	for(int i = 0; i < num_output_nodes; i++) {
		for(int j = 0; j < 4; j++) result_file_statistics[i][j] = 0;
	}
	for(int micro: microavg) micro = 0;
	for(double macro: macroavg) macro = 0;

	// retrieve weights
	for(int i = 0; i < num_hidden_nodes; i++) {
		for(int j = 0; j < num_input_nodes + 1; j++) {
			(j == num_input_nodes) ? getline(ifs, buffer) :	getline(ifs, buffer, delimiter);
			w_input_hidden[i][j] = stod(buffer);
		}
	}
	for(int i = 0; i < num_output_nodes; i++) {
		for(int j = 0; j < num_hidden_nodes + 1; j++) {
			(j == num_hidden_nodes) ? getline(ifs,buffer) : getline(ifs, buffer, delimiter);
			w_hidden_output[i][j] = stod(buffer);
		}
	}
	cout << "[INFO] All weights have been initialized from the trained network: " << init_file << endl;
	ifs.close();
	/* 1. Network weight initialization from file END */

	/* 2. Load test file and verify node sizes START */
	while(1) {
		cout << "Enter test file: ";
		cin >> test_file;
		ifs.open(test_file.c_str());
		// exit if file is opened
		if(ifs.is_open()) {
			break;
		} else {
			cerr << "[Error] Missing file: " << init_file << endl;
		}
	} 

	// parse first line
	getline(ifs, buffer, delimiter);
	num_train_examples = stoi(buffer);

	getline(ifs, buffer, delimiter);
	num_input_nodes_test = stoi(buffer);

	getline(ifs, buffer);
	num_output_nodes_test = stoi(buffer);

	if(num_input_nodes_test != num_input_nodes) {
		cerr <<	"Fatal Error: num_input_nodes_test of test_file file does not match init_file file" << endl;
		return -1;
	}
	if(num_output_nodes_test != num_output_nodes) {
		cerr <<	"Fatal Error: num_output_nodes_test of test_file file does not match init_file file" << endl;
		return -1;
	}
	/* 2. Load test file and verify node sizes END */

	/* 3. Backpropagation START */
	for(int i = 0; i < num_train_examples; i++) {
		// get weights from input to hidden nodes
		for(int j = 0; j < num_input_nodes; j++) {
			getline(ifs,buffer, delimiter);
			w_input_hidden2[j] = stod(buffer);
		}

		// get output values
		for(int j = 0; j < num_output_nodes; j++) {
			(j==num_output_nodes-1) ? getline(ifs,buffer):getline(ifs,buffer, delimiter);
			output_boolean[j] = stoi(buffer);
		}

		// {for l=2 to L do} part -- broken into hidden and output layers
		// loop through hidden layer nodes
		for(int j = 0; j < num_hidden_nodes; j++) {
			double sum = -w_input_hidden[j][0];
			for(int k = 1; k < num_input_nodes + 1; k++) {
				sum += w_input_hidden[j][k] * w_input_hidden2[k-1];
			}
			hidden_a[j] = sigmoid(sum);
		}
		// loop through output layer nodes
		for(int j = 0; j < num_output_nodes; j++) {
			double sum = -w_hidden_output[j][0];
			for(int k = 1; k < num_hidden_nodes + 1; k++) {
				sum += w_hidden_output[j][k] * hidden_a[k-1];
			}
			output_a[j] = sigmoid(sum);

			// activation
			int predicted_boolean = (output_a[j] >= 0.5) ? 1 : 0;
			if (predicted_boolean && output_boolean[j]) {  // True positive
				result_file_statistics[j][0]++; // a
			}
			else if (predicted_boolean && !output_boolean[j]) { // False Negative
				result_file_statistics[j][1]++; // b
			}
			else if (!predicted_boolean && output_boolean[j]) { // False Positive
				result_file_statistics[j][2]++; // c 
			}
			else if (!predicted_boolean && !output_boolean[j]) { // True Negative
				result_file_statistics[j][3]++; // d
			}
		}
	}
	ifs.close();	
	/* 3. Backpropagation END */

	/* 4. Save tested outputs START */
	while(1) {
		cout << "Enter test result_file filename (to be saved as): ";
		cin >> result_file;
		ofs.open(result_file.c_str());
		// exit if file is opened
		if(ofs.is_open()) {
			break;
		} else {
			cerr << "[Error] file descriptor failed to open for filename: " << result_file << endl;
		}
	} 
	// compute statistical metrics
	ofs << setprecision(3) << fixed;
	for(int i = 0; i < num_output_nodes; i++) {
		// micro-averaging
		a = result_file_statistics[i][0];
		b = result_file_statistics[i][1];
		c = result_file_statistics[i][2];
		d = result_file_statistics[i][3];
		microavg[0] += a;
		microavg[1] += b;
		microavg[2] += c;
		microavg[3] += d;
		ofs << a << " " << b << " " << c << " " << d << " ";

		// macro-averaging
		accuracy = (double)(a + d) / (double)(a + b + c + d);
		precision = (double)(a) / (double)(a + b);
		recall = (double)(a) / (double)(a + c);
		f1 = (2 * precision * recall) / (precision + recall);
		macroavg[0] += accuracy;
		macroavg[1] += precision;
		macroavg[2] += recall;
		ofs << accuracy << " " << precision << " " << recall << " " << f1 <<endl;
	}

	accuracy = (double) (microavg[0] + microavg[3]) / (microavg[0] + microavg[1] + microavg[2]+ microavg[3]);
	precision = (double) (microavg[0]) / (microavg[0] + microavg[1]);
	recall = (double) (microavg[0]) / (microavg[0] + microavg[2]);
	f1 = (double)  (2 * precision * recall) / (precision+recall);
	ofs << accuracy << " " << precision << " " << recall << " " << f1 <<endl;

	accuracy = macroavg[0] / num_output_nodes;
	precision = macroavg[1] / num_output_nodes;
	recall = macroavg[2] / num_output_nodes;
	f1 = 2 * precision * recall / (precision + recall);
	ofs << accuracy << " " << precision << " " << recall << " " << f1 <<endl;

	ofs.close();
	/* 4. Save tested outputs END*/
}

double sigmoid(const double &z) {
	return 1.0 / (1.0 + exp(-1.0 * z));
}
