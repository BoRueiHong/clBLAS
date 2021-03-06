/*******************************************************************************
 * Hand-tuned kernel
 *
 ******************************************************************************/

#ifndef KERNEL_TRIPLE_DGEMM_UPDATE_128_16_PART1_L_SRC_CPP
#define KERNEL_TRIPLE_DGEMM_UPDATE_128_16_PART1_L_SRC_CPP

#ifndef STRINGIFY
#define STRINGIFY2(...) #__VA_ARGS__
#define STRINGIFY(...) STRINGIFY2(__VA_ARGS__)
#endif

unsigned char *triple_dgemm_update_128_16_PART1_L_bin = 0;
size_t triple_dgemm_update_128_16_PART1_L_binSize = 0;

const char * const triple_dgemm_update_128_16_PART1_L_src = STRINGIFY(
static void daxpy(\n
double alpha, \n
__local const double * __restrict__ b, \n
double * __restrict__ c)\n
{ \n
c[0] += alpha * b[0]; \n
c[1] += alpha * b[1]; \n
c[2] += alpha * b[2]; \n
c[3] += alpha * b[3]; \n
c[4] += alpha * b[4]; \n
c[5] += alpha * b[5]; \n
c[6] += alpha * b[6]; \n
c[7] += alpha * b[7]; \n
c[8] += alpha * b[8]; \n
c[9] += alpha * b[9]; \n
c[10] += alpha * b[10]; \n
c[11] += alpha * b[11]; \n
c[12] += alpha * b[12]; \n
c[13] += alpha * b[13]; \n
c[14] += alpha * b[14]; \n
c[15] += alpha * b[15]; \n
}\n
#define NB 128\n
#define __mul(i,j) ((i)*(j))\n
#define qmod(a, b) ((a)%(b))\n
	__kernel void TRIPLE_DGEMM_UPDATE_128_16_PART1_L(__global const double *Ain, uint offAin, __global double *d_dinvA, int blk, uint lda, int npages, int na)\n
{ \n
const int bIdy = get_group_id(1) / npages;\n
//const int page = (get_group_id(1))%(npages);
const int page = qmod(get_group_id(1), npages); \n
const int inx = get_local_id(0); \n
const int iny = get_local_id(1); \n
const int ibx = get_group_id(0) * (get_local_size(0)*get_local_size(1)); \n
const int iby = bIdy * 16; \n
const int id = inx + iny*get_local_size(0); \n
__local double bs[16][17]; \n

Ain = Ain + offAin; \n

//--------------------------part one---------------------------//
{
	// A21*inv(A11) -> A21
	// A=A21, B=inv(A11), C=A21
	__global const double *A; \n
	__global double *B, *C; \n
	int ldb = NB; \n
	int ldc = NB; \n

	int PagesPerNB = NB / (blk * 2); \n

	d_dinvA += NB*NB*(page / PagesPerNB)
		+ (qmod(page, PagesPerNB))*(blk * 2)*NB
		+ (qmod(page, PagesPerNB))*(blk * 2); \n

	int xa = page*blk * 2 + blk + ibx + id; \n
	int ya = page*blk * 2; \n
	int incA = ya * lda + xa; \n

	// maxA will be used to detect overflow on all subsequent accesses on A(xa, ya:ya+???)

	int maxA; \n
	if (xa < na)\n
		maxA = lda*na; \n // macro READA will detect overflow on y dimension
	else\n
	    maxA = 0; \n  // there is already an overflow on xa

#define READA ( (incA < maxA ) ? Ain[incA] : 0 )  \n

	B = d_dinvA; \n
	C = d_dinvA + blk; \n

	B += inx + __mul(iby + iny, ldb); \n
	C += ibx + id + __mul(iby, ldc); \n

	__global double *Blast = B + blk; \n

	double c[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; \n

	do {\n
		double a[4]; \n
		a[0] = ((incA < maxA) ? Ain[incA] : 0); incA += lda; \n
		a[1] = ((incA < maxA) ? Ain[incA] : 0); incA += lda; \n
		a[2] = ((incA < maxA) ? Ain[incA] : 0); incA += lda; \n
		a[3] = ((incA < maxA) ? Ain[incA] : 0); incA += lda; \n

		bs[inx][iny] = B[0 * ldb]; \n
		bs[inx][iny + 4] = B[4 * ldb]; \n
		bs[inx][iny + 8] = B[8 * ldb]; \n
		bs[inx][iny + 12] = B[12 * ldb]; \n
		bs[inx + 4][iny] = B[4 + 0 * ldb]; \n
		bs[inx + 4][iny + 4] = B[4 + 4 * ldb]; \n
		bs[inx + 4][iny + 8] = B[4 + 8 * ldb]; \n
		bs[inx + 4][iny + 12] = B[4 + 12 * ldb]; \n
		bs[inx + 8][iny] = B[8 + 0 * ldb]; \n
		bs[inx + 8][iny + 4] = B[8 + 4 * ldb]; \n
		bs[inx + 8][iny + 8] = B[8 + 8 * ldb]; \n
		bs[inx + 8][iny + 12] = B[8 + 12 * ldb]; \n
		bs[inx + 12][iny] = B[12 + 0 * ldb]; \n
		bs[inx + 12][iny + 4] = B[12 + 4 * ldb]; \n
		bs[inx + 12][iny + 8] = B[12 + 8 * ldb]; \n
		bs[inx + 12][iny + 12] = B[12 + 12 * ldb]; \n
		//__syncthreads();
		barrier(CLK_LOCAL_MEM_FENCE); \n

		daxpy(a[0], &bs[0][0], c);  a[0] = ((incA < maxA) ? Ain[incA] : 0); incA += lda; \n
		daxpy(a[1], &bs[1][0], c);  a[1] = ((incA < maxA) ? Ain[incA] : 0); incA += lda; \n
		daxpy(a[2], &bs[2][0], c);  a[2] = ((incA < maxA) ? Ain[incA] : 0); incA += lda; \n
		daxpy(a[3], &bs[3][0], c);  a[3] = ((incA < maxA) ? Ain[incA] : 0); incA += lda; \n

		daxpy(a[0], &bs[4][0], c);  a[0] = ((incA < maxA) ? Ain[incA] : 0); incA += lda; \n
		daxpy(a[1], &bs[5][0], c);  a[1] = ((incA < maxA) ? Ain[incA] : 0); incA += lda; \n
		daxpy(a[2], &bs[6][0], c);  a[2] = ((incA < maxA) ? Ain[incA] : 0); incA += lda; \n
		daxpy(a[3], &bs[7][0], c);  a[3] = ((incA < maxA) ? Ain[incA] : 0); incA += lda; \n

		daxpy(a[0], &bs[8][0], c);  a[0] = ((incA < maxA) ? Ain[incA] : 0); incA += lda; \n
		daxpy(a[1], &bs[9][0], c);  a[1] = ((incA < maxA) ? Ain[incA] : 0); incA += lda; \n
		daxpy(a[2], &bs[10][0], c);  a[2] = ((incA < maxA) ? Ain[incA] : 0); incA += lda; \n
		daxpy(a[3], &bs[11][0], c);  a[3] = ((incA < maxA) ? Ain[incA] : 0); incA += lda; \n

		daxpy(a[0], &bs[12][0], c); \n
		daxpy(a[1], &bs[13][0], c); \n
		daxpy(a[2], &bs[14][0], c); \n
		daxpy(a[3], &bs[15][0], c); \n

		B += 16; \n
		//__syncthreads();
		barrier(CLK_LOCAL_MEM_FENCE); \n
	} while (B < Blast); \n

	for (int i = 0; i < 16; i++) {\n
		C[0] = c[i]; \n
		C += ldc; \n
	}\n
}\n

#undef READA\n

//__syncthreads();
barrier(CLK_LOCAL_MEM_FENCE); \n
}\n
// end of kernel
);
#endif
