#include <cmath>
#include <iomanip>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

static double sigmoid(const double &z);
static double sigmoid_derivative(const double &z);

int main() {
	string init_file, train_file, result_file;
	ifstream ifs;
	ofstream ofs;
	char delimiter = ' ';
	int num_input_nodes, num_hidden_nodes, num_output_nodes;

	while(1) {
		cout << "Enter filename for initializing network: ";
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
	double hidden_in[num_hidden_nodes];
	double hidden_a[num_hidden_nodes];
	double hidden_delta[num_hidden_nodes];
	double output_in[num_output_nodes];
	double output_a[num_output_nodes];
	double output_delta[num_output_nodes];
	int output_boolean[num_output_nodes];
	
	/* 1. Network weight initialization from file START */
	// fill in hidden node weights
	for(int i = 0; i < num_hidden_nodes; i++) {
		for(int j = 0; j < num_input_nodes + 1; j++) {
			(j == num_input_nodes) ? getline(ifs, buffer) : getline(ifs, buffer, delimiter);
			w_input_hidden[i][j] = stod(buffer);
		}
	}

	// fill in output node weights
	for(int i = 0; i < num_output_nodes; i++) {
		for(int j = 0; j < num_hidden_nodes + 1; j++) {
			(j == num_hidden_nodes) ? getline(ifs, buffer) : getline(ifs, buffer, delimiter);
			w_hidden_output[i][j] = stod(buffer);
		}
	}
	cout << "[INFO] All weights have been initialized from file: " << init_file << endl;
	ifs.close();
	/* 1. Network weight initialization from file END */

	/* 2. Weights of hidden nodes from training file START */
	while(1) {
		cout << "Enter training file: ";
		cin >> train_file;
		ifs.open(train_file.c_str());
		// exit if file is opened
		if(ifs.is_open()) {
			break;
		} else {
			cerr << "[Error] Missing file: " << init_file << endl;
		}
	} 

	int epoch = 100;
	double learning_rate = 0.05;

	// take epoch value
	do {
		cout << "Enter epoch (positive integer): ";
		if(cin >> epoch) break;
		cout << "[Error] invalid epoch value (needs to be an integer)" << endl;
	} while(1);

	// take learning rate value
	do {
		cout << "Enter learning rate (double): ";
		if(cin >> learning_rate) break;
		cout << "[Error] invalid learning rate value (needs to be a double)" << endl;
	} while(1);
	
	// check the number of input and output nodes of init and train files
	int num_train_examples, num_input_nodes_train, num_output_nodes_train;
	
	// parse first line
	getline(ifs, buffer, delimiter);
	num_train_examples = stoi(buffer);

	getline(ifs, buffer, delimiter);
	num_input_nodes_train = stoi(buffer);

	getline(ifs, buffer);
	num_output_nodes_train = stoi(buffer);

	// check if input layer size for initialized network matches the specified train input layer size
	if(num_input_nodes_train != num_input_nodes) {
		cerr <<	"[Error] Input node numbers do not match between init and train" << endl;
		return -1;
	}
	if(num_output_nodes_train != num_output_nodes) {
		cerr <<	"[Error] Output node numbers do not match between init and train" << endl;
		return -1;
	}
	ifs.close();
	/* 2. Weights of hidden nodes from training file END */


	/* 3. Backpropagation START */
	// repeat for 'epoch' number of times
	for (int i = 0; i < epoch; i++) {
		ifs.open(train_file.c_str());
		getline(ifs, buffer);
		for(int j = 0; j < num_train_examples; j++) {
			// get weights from input to hidden nodes
			for(int k = 0; k < num_input_nodes; k++) {
				getline(ifs, buffer, delimiter);
				w_input_hidden2[k] = stod(buffer);
			}

			// get output values
			for(int k = 0; k < num_output_nodes; k++) {
				(k==num_output_nodes-1) ? getline(ifs, buffer) : getline(ifs, buffer, delimiter);
				output_boolean[k] = stoi(buffer);
			}

			// {for l=2 to L do} part -- broken into hidden and output layers
			// loop through hidden layer nodes
			for(int k = 0; k < num_hidden_nodes; k++) {
				double sum = -w_input_hidden[k][0];
				for(int l = 1; l < num_input_nodes + 1; l++) {
					sum += w_input_hidden[k][l] * w_input_hidden2[l-1];
				}
				hidden_in[k] = sum;
				hidden_a[k] = sigmoid(sum);
			}
			// loop through output layer nodes
			for(int k = 0; k < num_output_nodes; k++) {
				double sum = -w_hidden_output[k][0];
				for(int l = 1; l < num_hidden_nodes + 1; l++) {
					sum += w_hidden_output[k][l] * hidden_a[l-1];
				}
				output_in[k] = sum;
				output_a[k] = sigmoid(sum);
			}

			// propagate deltas backward from output layer to input layer
			for(int k = 0; k < num_output_nodes; k++) {
				output_delta[k] = sigmoid_derivative(output_in[k]) * (output_boolean[k] - output_a[k]);
			}
			for(int k = 0; k < num_hidden_nodes; k++) {
				double sum2 = 0;
				for(int l = 0; l < num_output_nodes; l++) {
					sum2 += w_hidden_output[l][k+1] * output_delta[l];
				}
				hidden_delta[k] = sigmoid_derivative(hidden_in[k]) * sum2;
			}

			// {for l=L-1 to 2 do} part -- broken into hidden and output layers
			// loop through hidden layer nodes to update every weight in network using deltas
			for(int k = 0; k < num_hidden_nodes; k++) {
				w_input_hidden[k][0] = w_input_hidden[k][0] + learning_rate * -1.0 * hidden_delta[k];
				for(int l = 1; l < num_input_nodes + 1; l++) {
					w_input_hidden[k][l] = w_input_hidden[k][l] + learning_rate * w_input_hidden2[l-1] * hidden_delta[k];
				} 
			}
			// loop through output layer nodes to update every weight in network using deltas
			for(int k = 0; k < num_output_nodes; k++) {
				w_hidden_output[k][0] = w_hidden_output[k][0] + learning_rate * -1.0 * output_delta[k];
				for(int l = 1; l < num_hidden_nodes + 1; l++) {
					w_hidden_output[k][l] = w_hidden_output[k][l] + learning_rate * hidden_a[l-1] * output_delta[k]; 
				} 
			}
		}
		ifs.close();
	}
	cout << "[INFO] Training completed after " << epoch <<  " epochs." << endl;
	/* 3. Backpropagation END */

	/* 4. Save trained network START */
	while(1) {
		cout << "Enter trained network filename (to be saved as): ";
		cin >> result_file;
		ofs.open(result_file.c_str());
		// exit if file is opened
		if(ofs.is_open()) {
			break;
		} else {
			cerr << "[Error] file descriptor failed to open for filename: " << result_file << endl;
		}
	} 
	/* 4. Save trained network END */

	/* 5. save weights from training START */
	ofs << setprecision(3) << fixed;
	// first line: # input nodes, # hidden nodes, # output nodes
	ofs << num_input_nodes << " " << num_hidden_nodes << " " << num_output_nodes << endl;

	// next lines: weights from input to hidden nodes
	for(int i = 0; i < num_hidden_nodes; i++) {
		for(int j = 0; j < num_input_nodes; j++) {
			ofs << w_input_hidden[i][j] << " ";
		}
		ofs << w_input_hidden[i][num_input_nodes] << endl;
	}

	// next lines: weights from hidden to output nodes
	for(int i = 0; i < num_output_nodes; i++) {
		for(int j = 0; j < num_hidden_nodes; j++) {
			ofs << w_hidden_output[i][j] << " ";
		}
		ofs << w_hidden_output[i][num_hidden_nodes] << endl;
	}
	ofs.close();
	/* 5. save weights from training END */
}


static double sigmoid(const double &z) {
	return 1.0 / (1.0 + exp(-1.0 * z));
}

static double sigmoid_derivative(const double &z) {
	return sigmoid(z) * (1 - sigmoid(z));
}
