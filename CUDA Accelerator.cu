#include "CUDA Accelerator.h"

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

using namespace std;


__global__ void TranslateKernel(float *pPosition, float trans_x, float trans_y, float trans_z, int size, int GridScale)
{
	// Current thread position
	int Row = blockIdx.y * TILE_WIDTH + threadIdx.y;
	int Col = blockIdx.x * TILE_WIDTH + threadIdx.x;

	int rowLength = GridScale * TILE_WIDTH;
	unsigned int index = Row*rowLength + Col;

	if (index < size * 3) { // Within available data
		int rest = index % 3; // Determine dimension
		if (rest == 0) pPosition[index] = pPosition[index] + trans_x;
		else if (rest == 1) pPosition[index] = pPosition[index] + trans_y;
		else if (rest == 2) pPosition[index] = pPosition[index] + trans_z;
	}
}

__global__ void RotateKernel(float *pPosition, float *pPosition_ori, float *pNormal, float *pNormal_ori, float *rotate_mat, int size, int GridScale)
{
	int Row = blockIdx.y * TILE_WIDTH + threadIdx.y;
	int Col = blockIdx.x * TILE_WIDTH + threadIdx.x;

	int rowLength = GridScale * TILE_WIDTH;
	unsigned int index = Row*rowLength + Col;

	if (index < size * 4) {
		int rest = index % 4;
		if (rest == 0) {
			pPosition[index] = // Rotate vertices
				pPosition_ori[index] * rotate_mat[0]
				 + pPosition_ori[index + 1] * rotate_mat[1]
				 + pPosition_ori[index + 2] * rotate_mat[2]
				 + pPosition_ori[index + 3] * rotate_mat[3];
			pNormal[index] =   // Rotate normals
				pNormal_ori[index] * rotate_mat[0]
				+ pNormal_ori[index + 1] * rotate_mat[1]
				+ pNormal_ori[index + 2] * rotate_mat[2]
				+ pNormal_ori[index + 3] * rotate_mat[3];
		}
		else if (rest == 1) {
			pPosition[index] = 
				pPosition_ori[index - 1] * rotate_mat[4]
				+ pPosition_ori[index] * rotate_mat[5]
				+ pPosition_ori[index + 1] * rotate_mat[6]
				+ pPosition_ori[index + 2] * rotate_mat[7];
			pNormal[index] = 
				pNormal_ori[index - 1] * rotate_mat[4]
				+ pNormal_ori[index] * rotate_mat[5]
				+ pNormal_ori[index + 1] * rotate_mat[6]
				+ pNormal_ori[index + 2] * rotate_mat[7];
		}
		else if (rest == 2) {
			pPosition[index] = 
				pPosition_ori[index - 2] * rotate_mat[8]
				+ pPosition_ori[index - 1] * rotate_mat[9]
				+ pPosition_ori[index] * rotate_mat[10]
				+ pPosition_ori[index + 1] * rotate_mat[11];
			pNormal[index] = 
				pNormal_ori[index - 2] * rotate_mat[8]
				+ pNormal_ori[index - 1] * rotate_mat[9]
				+ pNormal_ori[index] * rotate_mat[10]
				+ pNormal_ori[index + 1] * rotate_mat[11];
		}
		else if (rest == 3) {
			pPosition[index] = 
				pPosition_ori[index - 3] * rotate_mat[12]
				+ pPosition_ori[index - 2] * rotate_mat[13]
				+ pPosition_ori[index - 1] * rotate_mat[14]
				+ pPosition_ori[index] * rotate_mat[15];
			pNormal[index] =
				pNormal_ori[index - 3] * rotate_mat[12]
				+ pNormal_ori[index - 2] * rotate_mat[13]
				+ pNormal_ori[index - 1] * rotate_mat[14]
				+ pNormal_ori[index] * rotate_mat[15];
		}
	}
}

__global__ void RescaleKernel(float *pPosition, float scale_x, float scale_y, float scale_z, int size, int GridScale)
{
	int Row = blockIdx.y * TILE_WIDTH + threadIdx.y;
	int Col = blockIdx.x * TILE_WIDTH + threadIdx.x;

	int rowLength = GridScale * TILE_WIDTH;
	unsigned int index = Row*rowLength + Col;

	if (index < size * 3) {
		int rest = index % 3;
		if (rest == 0) pPosition[index] = pPosition[index] * scale_x;
		else if (rest == 1) pPosition[index] = pPosition[index] * scale_y;
		else if (rest == 2) pPosition[index] = pPosition[index] * scale_z;
	}
}

__global__ void ShiftUVKernel(float *pPosition, float shift_x, float shift_y, int size, int GridScale)
{
	int Row = blockIdx.y * TILE_WIDTH + threadIdx.y;
	int Col = blockIdx.x * TILE_WIDTH + threadIdx.x;

	int rowLength = GridScale * TILE_WIDTH;
	unsigned int index = Row*rowLength + Col;

	if (index < size * 2) {
		int rest = index % 2;
		if (rest == 0) pPosition[index] = pPosition[index] + shift_x;
		else if (rest == 1) pPosition[index] = pPosition[index] + shift_y;
	}
}

__global__ void RescaleUVKernel(float *pPosition, float scale_x, float scale_y, int size, int GridScale)
{
	int Row = blockIdx.y * TILE_WIDTH + threadIdx.y;
	int Col = blockIdx.x * TILE_WIDTH + threadIdx.x;

	int rowLength = GridScale * TILE_WIDTH;
	unsigned int index = Row*rowLength + Col;

	if (index < size * 2) {
		int rest = index % 2;
		if (rest == 0) pPosition[index] = pPosition[index] * scale_x;
		else if (rest == 1) pPosition[index] = pPosition[index] * scale_y;
	}
}

__global__ void RotateUVKernel(float *pPosition, float* pPosition_ori, float *rotate_mat, int size, int GridScale)
{
	int Row = blockIdx.y * TILE_WIDTH + threadIdx.y;
	int Col = blockIdx.x * TILE_WIDTH + threadIdx.x;

	int rowLength = GridScale * TILE_WIDTH;
	unsigned int index = Row*rowLength + Col;

	if (index < size * 2) {
		int rest = index % 2;
		if (rest == 0) {
			pPosition[index] =
				pPosition_ori[index] * rotate_mat[0]
				+ pPosition_ori[index + 1] * rotate_mat[1];
		}
		else if (rest == 1) {
			pPosition[index] =
				pPosition_ori[index - 1] * rotate_mat[2]
				+ pPosition_ori[index] * rotate_mat[3];
		}
	}
}

__global__ void STG_TranslateKernel(float *pPosition_y, float trans_y, int size, int GridScale)
{
	int Row = blockIdx.y * TILE_WIDTH + threadIdx.y;
	int Col = blockIdx.x * TILE_WIDTH + threadIdx.x;

	int rowLength = GridScale * TILE_WIDTH;
	unsigned int index = Row*rowLength + Col;

	if (index < size) {
		pPosition_y[index] = pPosition_y[index] + trans_y;
	}
}


extern "C" void translate_CUDA(vector<Vertex> &vertices, vec4 translation)
{
	cudaSetDevice(0); // Select CUDA device

	int verticesSize = vertices.size();
	// Device array
	float *pPosition;
	unsigned int size = verticesSize * 3 * sizeof(float);
	cudaMalloc((void**)&pPosition, size);

	// Temporary host transfer arrays
	float *temp_Position;
	temp_Position = new float[verticesSize * 3];
	for (int i = 0; i < verticesSize; i++) {
		for (int j = 0; j < 3; j++) {
			temp_Position[3 * i + j] = vertices[i].position[j];
		}
	}

	// Copy data to device
	cudaMemcpy(pPosition, temp_Position, size, cudaMemcpyHostToDevice);

	int gridScale = ceil(sqrt(verticesSize * 3 / (TILE_WIDTH*TILE_WIDTH)));
	if (gridScale < 1) gridScale = 1; // If the data is too little
	dim3 dimGrid(gridScale, gridScale); // Dimension of a grid
	dim3 dimBlock(TILE_WIDTH, TILE_WIDTH); // Dimension of a block

	float trans_x = translation.x; float trans_y = translation.y; float trans_z = translation.z;

	// Call kernel function
	TranslateKernel <<< dimGrid, dimBlock >>> (pPosition, trans_x, trans_y, trans_z, verticesSize, gridScale);

	// Copy data to host
	cudaMemcpy(temp_Position, pPosition, size, cudaMemcpyDeviceToHost);

	// Update real data
	for (int i = 0; i < verticesSize; i++) {
		for (int j = 0; j < 3; j++) {
			vertices[i].position[j] = temp_Position[3 * i + j];
		}
	}

	// Release memory
	cudaFree(pPosition);
	delete temp_Position;
}

extern "C" void rotate_CUDA(vector<Vertex> &vertices, mat4 rotation)
{
	cudaSetDevice(0);

	int verticesSize = vertices.size();

	float *pPosition;
	unsigned int size = verticesSize * 4 * sizeof(float);
	cudaMalloc((void**)&pPosition, size);

	float *pPosition_ori;
	cudaMalloc((void**)&pPosition_ori, size);

	float *temp_Position;
	temp_Position = new float[verticesSize * 4];
	for (int i = 0; i < verticesSize; i++) {
		for (int j = 0; j < 4; j++) {
			temp_Position[4 * i + j] = vertices[i].position[j];
		}
	}

	cudaMemcpy(pPosition, temp_Position, size, cudaMemcpyHostToDevice);

	cudaMemcpy(pPosition_ori, temp_Position, size, cudaMemcpyHostToDevice);

	// Normals
	float *pNormal;
	cudaMalloc((void**)&pNormal, size);

	float *pNormal_ori;
	cudaMalloc((void**)&pNormal_ori, size);

	float *temp_Normal;
	temp_Normal = new float[verticesSize * 4];
	for (int i = 0; i < verticesSize; i++) {
		for (int j = 0; j < 4; j++) {
			temp_Normal[4 * i + j] = vertices[i].normal[j];
		}
	}

	cudaMemcpy(pNormal, temp_Normal, size, cudaMemcpyHostToDevice);

	cudaMemcpy(pNormal_ori, temp_Normal, size, cudaMemcpyHostToDevice);

	// Transform matrix into array
	float *pMatrix;
	int m_size = 16 * sizeof(float);
	cudaMalloc((void**)&pMatrix, m_size);

	float *temp_Matrix;
	temp_Matrix = new float[16];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			temp_Matrix[4 * i + j] = rotation[i][j];
		}
	}
	cudaMemcpy(pMatrix, temp_Matrix, m_size, cudaMemcpyHostToDevice);

	int gridScale = ceil(sqrt(verticesSize * 4 / (TILE_WIDTH*TILE_WIDTH)));
	if (gridScale < 1) gridScale = 1;
	dim3 dimGrid(gridScale, gridScale);
	dim3 dimBlock(TILE_WIDTH, TILE_WIDTH);

	RotateKernel <<< dimGrid, dimBlock >>> (pPosition, pPosition_ori, pNormal, pNormal_ori, pMatrix, verticesSize, gridScale);

	cudaMemcpy(temp_Position, pPosition, size, cudaMemcpyDeviceToHost);

	for (int i = 0; i < verticesSize; i++) {
		for (int j = 0; j < 4; j++) {
			vertices[i].position[j] = temp_Position[4 * i + j];
		}
	}

	cudaMemcpy(temp_Normal, pNormal, size, cudaMemcpyDeviceToHost);

	for (int i = 0; i < verticesSize; i++) {
		for (int j = 0; j < 4; j++) {
			vertices[i].normal[j] = temp_Normal[4 * i + j];
		}
	}

	cudaFree(pPosition);
	cudaFree(pPosition_ori);
	cudaFree(pNormal);
	cudaFree(pNormal_ori);
	cudaFree(pMatrix);
	delete temp_Position;
	delete temp_Normal;
	delete temp_Matrix;
}

extern "C" void rescale_CUDA(vector<Vertex> &vertices, vec4 scale)
{
	cudaSetDevice(0);

	int verticesSize = vertices.size();

	float *pPosition;
	unsigned int size = verticesSize * 3 * sizeof(float);
	cudaMalloc((void**)&pPosition, size);

	float *temp_Position;
	temp_Position = new float[verticesSize * 3];
	for (int i = 0; i < verticesSize; i++) {
		for (int j = 0; j < 3; j++) {
			temp_Position[3 * i + j] = vertices[i].position[j];
		}
	}

	cudaMemcpy(pPosition, temp_Position, size, cudaMemcpyHostToDevice);

	int gridScale = ceil(sqrt(verticesSize * 3 / (TILE_WIDTH*TILE_WIDTH)));
	if (gridScale < 1) gridScale = 1;
	dim3 dimGrid(gridScale, gridScale);
	dim3 dimBlock(TILE_WIDTH, TILE_WIDTH);

	float scale_x = scale.x; float scale_y = scale.y; float scale_z = scale.z;

	RescaleKernel <<< dimGrid, dimBlock >>> (pPosition, scale_x, scale_y, scale_z, verticesSize, gridScale);

	cudaMemcpy(temp_Position, pPosition, size, cudaMemcpyDeviceToHost);

	for (int i = 0; i < verticesSize; i++) {
		for (int j = 0; j < 3; j++) {
			vertices[i].position[j] = temp_Position[3 * i + j];
		}
	}

	cudaFree(pPosition);
	delete temp_Position;
}

extern "C" void shiftUV_CUDA(vector<Vertex> &vertices, vec4 shift)
{
	cudaSetDevice(0);

	int verticesSize = vertices.size();

	float *pPosition;
	unsigned int size = verticesSize * 2 * sizeof(float);
	cudaMalloc((void**)&pPosition, size);

	float *temp_Position;
	temp_Position = new float[verticesSize * 2];
	for (int i = 0; i < verticesSize; i++) {
		for (int j = 0; j < 2; j++) {
			temp_Position[2 * i + j] = vertices[i].tex_coords[j];
		}
	}

	cudaMemcpy(pPosition, temp_Position, size, cudaMemcpyHostToDevice);

	int gridScale = ceil(sqrt(verticesSize * 2 / (TILE_WIDTH*TILE_WIDTH)));
	if (gridScale < 1) gridScale = 1;
	dim3 dimGrid(gridScale, gridScale);
	dim3 dimBlock(TILE_WIDTH, TILE_WIDTH);

	float shift_x = shift.x; float shift_y = shift.y;

	ShiftUVKernel <<< dimGrid, dimBlock >>> (pPosition, shift_x, shift_y, verticesSize, gridScale);

	cudaMemcpy(temp_Position, pPosition, size, cudaMemcpyDeviceToHost);

	for (int i = 0; i < verticesSize; i++) {
		for (int j = 0; j < 2; j++) {
			vertices[i].tex_coords[j] = temp_Position[2 * i + j];
		}
	}

	cudaFree(pPosition);
	delete temp_Position;
}

extern "C" void rescaleUV_CUDA(vector<Vertex> &vertices, vec4 scale)
{
	cudaSetDevice(0);

	int verticesSize = vertices.size();

	float *pPosition;
	unsigned int size = verticesSize * 2 * sizeof(float);
	cudaMalloc((void**)&pPosition, size);

	float *temp_Position;
	temp_Position = new float[verticesSize * 2];
	for (int i = 0; i < verticesSize; i++) {
		for (int j = 0; j < 2; j++) {
			temp_Position[2 * i + j] = vertices[i].tex_coords[j];
		}
	}

	cudaMemcpy(pPosition, temp_Position, size, cudaMemcpyHostToDevice);

	int gridScale = ceil(sqrt(verticesSize * 2 / (TILE_WIDTH*TILE_WIDTH)));
	if (gridScale < 1) gridScale = 1;
	dim3 dimGrid(gridScale, gridScale);
	dim3 dimBlock(TILE_WIDTH, TILE_WIDTH);

	float scale_x = scale.x; float scale_y = scale.y;

	RescaleUVKernel <<< dimGrid, dimBlock >>> (pPosition, scale_x, scale_y, verticesSize, gridScale);

	cudaMemcpy(temp_Position, pPosition, size, cudaMemcpyDeviceToHost);

	for (int i = 0; i < verticesSize; i++) {
		for (int j = 0; j < 2; j++) {
			vertices[i].tex_coords[j] = temp_Position[2 * i + j];
		}
	}

	cudaFree(pPosition);
	delete temp_Position;
}

extern "C" void rotateUV_CUDA(vector<Vertex> &vertices, float angle)
{
	cudaSetDevice(0);

	int verticesSize = vertices.size();

	float *pPosition;
	unsigned int size = verticesSize * 2 * sizeof(float);
	cudaMalloc((void**)&pPosition, size);

	float *pPosition_ori;
	cudaMalloc((void**)&pPosition_ori, size);

	float *temp_Position;
	temp_Position = new float[verticesSize * 2];
	for (int i = 0; i < verticesSize; i++) {
		for (int j = 0; j < 2; j++) {
			temp_Position[2 * i + j] = vertices[i].tex_coords[j];
		}
	}

	cudaMemcpy(pPosition, temp_Position, size, cudaMemcpyHostToDevice);

	cudaMemcpy(pPosition_ori, temp_Position, size, cudaMemcpyHostToDevice);

	float *temp_Matrix;
	temp_Matrix = new float[4];
	float c = cos(angle), s = sin(angle);
	temp_Matrix[0] = c; temp_Matrix[1] = s;
	temp_Matrix[2] = -s; temp_Matrix[3] = c;

	float *pMatrix;
	int m_size = 4 * sizeof(float);
	cudaMalloc((void**)&pMatrix, m_size);

	cudaMemcpy(pMatrix, temp_Matrix, m_size, cudaMemcpyHostToDevice);

	int gridScale = ceil(sqrt(verticesSize * 2 / (TILE_WIDTH*TILE_WIDTH)));
	if (gridScale < 1) gridScale = 1;
	dim3 dimGrid(gridScale, gridScale);
	dim3 dimBlock(TILE_WIDTH, TILE_WIDTH);

	RotateUVKernel <<< dimGrid, dimBlock >>> (pPosition, pPosition_ori, pMatrix, verticesSize, gridScale);

	cudaMemcpy(temp_Position, pPosition, size, cudaMemcpyDeviceToHost);

	for (int i = 0; i < verticesSize; i++) {
		for (int j = 0; j < 2; j++) {
			vertices[i].tex_coords[j] = temp_Position[2 * i + j];
		}
	}

	cudaFree(pPosition);
	cudaFree(pMatrix);
	cudaFree(pPosition_ori);
	delete temp_Position;
	delete temp_Matrix;
}

extern "C" void stg_translate_CUDA(vector<vec4> &positions, float compensation)
{
	cudaSetDevice(0);

	int verticesSize = positions.size();

	float *pPosition_y;
	unsigned int size = verticesSize * sizeof(float);
	cudaMalloc((void**)&pPosition_y, size);

	float *temp_Position_y;
	temp_Position_y = new float[verticesSize];
	for (int i = 0; i < verticesSize; i++) {
		temp_Position_y[i] = positions[i].y;
	}

	cudaMemcpy(pPosition_y, temp_Position_y, size, cudaMemcpyHostToDevice);

	int gridScale = ceil(sqrt(verticesSize / (TILE_WIDTH*TILE_WIDTH)));
	if (gridScale < 1) gridScale = 1;
	dim3 dimGrid(gridScale, gridScale);
	dim3 dimBlock(TILE_WIDTH, TILE_WIDTH);

	STG_TranslateKernel <<< dimGrid, dimBlock >>> (pPosition_y, compensation, verticesSize, gridScale);

	cudaMemcpy(temp_Position_y, pPosition_y, size, cudaMemcpyDeviceToHost);

	for (int i = 0; i < verticesSize; i++) {
		positions[i].y = temp_Position_y[i];
	}

	cudaFree(pPosition_y);
	delete temp_Position_y;
}