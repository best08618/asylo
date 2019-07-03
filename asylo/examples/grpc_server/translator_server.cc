/*
 *
 * Copyright 2018 Asylo authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "asylo/examples/grpc_server/translator_server.h"
#include "absl/strings/str_split.h" // jinhwan

#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/synchronization/mutex.h"
#include "include/grpcpp/grpcpp.h"

namespace examples {
namespace grpc_server {

TranslatorServer::TranslatorServer()
    : Service(),
      // Initialize the translation map with a few known translations.
      translation_map_({{"asylo", "sanctuary"},
                        {"istio", "sail"},
                        {"kubernetes", "helmsman"}}) {}
float** matrix1;
float** matrix2;
float** matrix_result;
absl::Mutex mutex_;
std::string output;

int row_mat1 = 0;
int col_mat1 = 0;

int row_mat2 = 0;
int col_mat2 = 0;

void TranslatorServer::split(const std::string &str, std::vector<std::string> &vect, char ch)
{
	int pos = str.find(ch);
	int initialPos = 0;
	int count = 0;
	vect.clear();

	while (pos != std::string::npos) {
		vect.push_back(str.substr(initialPos, pos - initialPos));
		count++;
		initialPos = pos + 1;

		pos = str.find(ch, initialPos);
	}
	int min = (pos < str.size()) ? pos : str.size();
	vect.push_back(str.substr(initialPos, min - initialPos + 1));
	count++;
}

float** TranslatorServer::getMat(std::string input, int &count_rows, int &count_columns) {
	float** ary;
	std::vector<std::string> vect;
	split(input, vect, ' ');

	std::string shape_val = vect[0];

	std::vector<std::string> vect_shape;
	split(shape_val, vect_shape, ',');

	count_rows = atoi(vect_shape[0].erase(0, 1).c_str());
	count_columns = atoi(vect_shape[1].erase((vect_shape[1].length()-1), 1).c_str());

	ary = new float*[count_rows];
	for (int i = 0; i < count_rows; ++i)
		ary[i] = new float[count_columns];

	std::vector<std::string> vect_values;
	split(input, vect_values, ']');

	for (int row = 0; row < count_rows; row++) {
		std::vector<std::string> temp_r;
		split(vect_values[row+1].c_str(), temp_r, '[');
		std::string values_val = temp_r[1];

		std::vector<std::string> temp_c;
		split(values_val, temp_c, ' ');

		for (int column = 0; column < count_columns; column++) {
			ary[row][column] = (float)atof(temp_c[column].c_str());
		}
	}

	return ary;
}

float** TranslatorServer::transpose(std::string input, int &count_rows, int &count_columns) {
	float** ary;
	std::vector<std::string> vect;
	split(input, vect, ' ');

	std::string shape_val = vect[0];

	std::vector<std::string> vect_shape;
	split(shape_val, vect_shape, ',');

	count_columns = atoi(vect_shape[0].erase(0, 1).c_str());
	count_rows = atoi(vect_shape[1].erase((vect_shape[1].length()-1), 1).c_str());

	ary = new float*[count_rows];
	for (int i = 0; i < count_rows; ++i)
		ary[i] = new float[count_columns];

	std::vector<std::string> vect_values;
	split(input, vect_values, ']');

	for (int column = 0; column < count_columns ; column++) {
		std::vector<std::string> temp_r;
		split(vect_values[column+1].c_str(), temp_r, '[');
		std::string values_val = temp_r[1];

		std::vector<std::string> temp_c;
		split(values_val, temp_c, ' ');

		for (int row = 0; row < count_rows ; row++) {
			ary[row][column] = (float)atof(temp_c[row].c_str());
		}
	}

	return ary;
}
float** TranslatorServer::matmul(float** input_mat1, float** input_mat2) {
	float** result_mat;

	if (col_mat1 != row_mat2) {
		std::cout << "ERROR" << std::endl;
		return NULL;
	}

	result_mat = new float*[row_mat1];

	for (int i = 0; i < row_mat1; i++)
	{
		result_mat[i] = new float[col_mat2];
	}

	float _temp=0;

	for (int i = 0; i < row_mat1; i++) {
		for (int j = 0; j < col_mat2; j++) {
			for (int k = 0; k < col_mat1; k++) {
				_temp += (input_mat1[i][k] * input_mat2[k][j]);
			}
			result_mat[i][j] = _temp;
			_temp = 0;
		}
	}
	return result_mat;
}
void TranslatorServer::getOutput() {
	//output = string("type: float32 shape: [");
	//output += std::to_string(row_mat1) + "," + std::to_string(col_mat2) + "] ";
	//output += "values: ";
	output = "";

	for (int i = 0; i < row_mat1; i++) {
		output += "[";
		for (int j = 0; j < col_mat2 -1; j++) {
			output += std::to_string(matrix_result[i][j])+" ";
		}
		output += std::to_string(matrix_result[i][col_mat2 -1]);
		output += "]";
	}
}
void TranslatorServer::deleteMemory() {
	for (int i = 0; i < row_mat1; i++)
	{
		delete[] matrix1[i];
	}
	delete[] matrix1;

	for (int i = 0; i < row_mat2; i++)
	{
		delete[] matrix2[i];
	}
	delete[] matrix2;

	for (int i = 0; i < row_mat1; i++)
	{
		delete[] matrix_result[i];
	}
	delete[] matrix_result;
}

::grpc::Status TranslatorServer::GetTranslation(
    ::grpc::ServerContext *context, const GetTranslationRequest *request,
    GetTranslationResponse *response) {
  // Confirm that |*request| has an |input_word| field.
  if (!request->has_input_word()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "No input word given");
  }

  // Confirm that the translation map has a translation for the input word.
  auto response_iterator =
      translation_map_.find(absl::AsciiStrToLower(request->input_word()));
  if (response_iterator == translation_map_.end()) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          absl::StrCat("No known translation for \"",
                                       request->input_word(), "\""));
  }

  // Return the translation.
  response->set_translated_word(response_iterator->second);
  return ::grpc::Status::OK;

}
::grpc::Status TranslatorServer::MatMul(
    ::grpc::ServerContext *context, const GetMatMulRequest *request,
    GetTranslationResponse *response) {
  // Confirm that |*request| has an |input_word| field.
  if (!request->has_tensor1() || !request->has_tensor2() ) {
    return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT,
                          "No input word given");
  }

	//mutex_.Lock();

	//request->input_word() should have two vectors
	//std::string input_str1 = "[1,3] [-2.5 0.5 1]";
	//std::string input_str2 = "[3,5] [1 1 1 1 1][1 0 3 0 5][0 2 0 4 0]";
	std::string input_str1 = request->tensor1();
	std::string input_str2 = request->tensor2();

	matrix1 = getMat(input_str1,row_mat1, col_mat1);
	matrix2 = getMat(input_str2,row_mat2, col_mat2);
	if (col_mat1 != row_mat2) {
		matrix2 = transpose(input_str2, row_mat2, col_mat2);
	}

	//std::cout << "input_data" << std::endl;
	//std::cout <<input_str1<< std::endl;
	//std::cout <<input_str2<< std::endl;

	matrix_result = matmul(matrix1, matrix2);
	
	if (matrix_result != NULL) {
		getOutput();
		//std::cout << "\noutput_data"<<std::endl;
		//std::cout << output<< std::endl;
	}
	else {
		std::cout << "matrix_result is NULL" << std::endl;
	}

	deleteMemory();

	response->set_translated_word(output);
	//mutex_.Unlock();
	return ::grpc::Status::OK;
  
}

}  // namespace grpc_server
}  // namespace examples 
