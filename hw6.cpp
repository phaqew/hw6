//part 0: head
#include <cstdlib>
#include "bitmap.cpp"
#include "fft.cpp"
inline int bound(double x){
	if(x < 0) return 0;
	if(x > 255) return 255;
	return round(x);
}
//part 1: produce gaussian noise
void addGaussianNoise(Bitmap *g, double mju, double sigma);
//part 2: spatial median filtering
void sptMidFtr(Bitmap *g, int z);
//part 3: spatial gaussian filtering
void sptGsnFtr(Bitmap *g, int z);
//part 4: frequency butterworth filtering
void frqBtwFtr(Bitmap *g, int r, int o);
//part 5: salt and pepper noise
void addSaltPepperNoise(Bitmap *g, double r);
//part 6: harmonic mean filtering
void hmnmFtr(Bitmap *g, int z, double Q);
//part 7: motion blur
void motionBlur(Bitmap *g, double a, double b, double T);
//part 8: inverser filtering
//part 9: wiener filtering


//part 1: produce gaussian noise
inline double gaussianRand() {
	return sqrt(-2.0*log((double)rand()/32768.0)) * cos((double)rand()*M_PI/32768.0);
}
void addGaussianNoise(Bitmap *g, double mju, double sigma) {
	for(int i = 0; i < g->height; i++) {
		for(int j = 0; j < g->width; j++) {
			double t =  g->px[i][j] + mju + sigma * gaussianRand();
			g->px[i][j] = bound(t);
		}
	}
}

//part 2: spatial median filtering
class segTreeC { //segment tree class for fast algorithm
	private:
		int m;
		int *n;
	public:
		segTreeC(int mm): m(mm) {
			n = new int[m + 1];
			for(int i = 0; i <= m; i++) n[i] = 0;
		}
		~segTreeC() {
			delete[] n;
		}
		void clr() {
			for(int i = 0; i <= m; i++) n[i] = 0;
		}
		void inc(int p) {
			n[++p]++;
			for(int t = 1; p < m; t <<= 1)
				if(p & t) n[p += t]++;
		}
		void dec(int p) {
			n[++p]--;
			for(int t = 1; p < m; t <<= 1)
				if(p & t) n[p += t]--;
		}
		int sum(int p) {
			int s = 0;
			for(int t = 0; p > 0; t++, p >>= 1)
				if(p & 1) s += n[p << t];
			return s;
		}
		int mid(int s) {
			int l = 0, r = 256;
			while(l < r - 1) {
				if(sum((l + r) / 2) < s) l = (l + r) / 2;
				else r = (l + r) / 2;
			}
			return l;
		}
};
void sptMidFtr(Bitmap *p, int z) {
	BMP256 g(p->height, p->width);
	segTreeC t(256);
	int m = z * z / 2 + 1;
	for(int y = z / 2; y < g.height - z / 2 - 1; y++) {
		t.clr();
		for(int i = y - z / 2; i <= y + z / 2; i++)
			for(int j = 0; j < z; j++) t.inc(p->px[i][j]);
		g.px[y][z / 2] = t.mid(m);
		for(int x = 1 + z / 2; x < g.width - z / 2 - 1; x++) {
			for(int i = y - z / 2; i <= y + z / 2; i++) {
				t.inc(p->px[i][x + z / 2]);
				t.dec(p->px[i][x - z / 2 - 1]);
			}
			g.px[y][x] = t.mid(m);
		}
	}
	for(int i = z / 2; i < g.height - z / 2 - 1; i++) {
		for(int j = z / 2; j < g.width - z / 2 - 1; j++) {
			p->px[i][j] = g.px[i][j];
		}
	}
}

//part 3: spatial gaussian filtering
#define sig -4.5 //-2*sigma^2
void sptGsnFtr(Bitmap *p, int z) {
	float gss = 0, **gs = new float*[z];
	for(int i = 0; i < z; i++) {
		gs[i] = new float[z];
		for(int j = 0; j < z; j++) {
			int d = (i - z / 2) * (i - z / 2) + (j - z / 2) * (j - z / 2);
			gs[i][j] = exp((float)d / sig);
			gss += gs[i][j];
		}
	}
	for(int i = 0; i < z; i++) for(int j = 0; j < z; j++) gs[i][j] /= gss;
	BMP256 g(p->height, p->width);
	for(int y = 0; y < g.height - z - 1; y++) {
		for(int x = 0; x < g.width - z - 1; x++) {
			float s = 0;
			for(int i = 0; i < z; i++) for(int j = 0; j < z; j++)
					s += (float)(p->px[y + i][x + j]) * gs[i][j];
			g.px[y + z / 2][x + z / 2] = (int)s;
		}
	}
	for(int i = z / 2; i < g.height - z / 2 - 2; i++) {
		for(int j = z / 2; j < g.width - z / 2 - 2; j++) {
			p->px[i][j] = g.px[i][j];
		}
	}
}

//part 4: frequency butterworth filtering
void frqBtwFtr(Bitmap *a, int radius, int order) {
	//initialize
	int n = 2 << (int)ceil(log2(a->height > a->width ? a->height : a->width));
	cmplx **x, **X;
	double **H;
	x = new cmplx*[n];
	X = new cmplx*[n];
	H = new double*[n];
	for(int i = 0; i < n; i++) {
		x[i] = new cmplx[n];
		X[i] = new cmplx[n];
		H[i] = new double[n];
		int u = i > n/2 ? n - i : i;
		for(int j = 0; j < n; j++) {
			int v = j > n/2 ? n - j : j;
			double d = (u*u + v*v)/radius/radius, dd = 1;
			for(int k = 0; k < order; k++) dd *= d;
			H[i][j] = 1.0 / (1.0 + dd);

			if(i < a->height && j < a->width) x[i][j].re = a->px[i][j];
			else x[i][j].re = 0;
		}
	}
	//2D FFT and frequency domain filtering
	FFT fft(n);
	fft.FFT2(x, X);
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < n; j++) {
			X[i][j].re *= H[i][j];
			X[i][j].im *= H[i][j];
		}
	}
	//IFFT & recover
	fft.FFT2(X, x);
	for(int i = 0; i < a->height; i++) {
		for(int j = 0; j < a->width; j++) {
			double m = x[n-i-1][n-j-1].re/n/n;
			if(m < 0) a->px[i][j] = 0;
			else if(m > 255) a->px[i][j] = 255;
			else a->px[i][j] = round(m);
		}
	}
}

//part 5: salt and pepper noise
void addSaltPepperNoise(Bitmap *g, double r) {
	for(int i = 0; i < g->height; i++) {
		for(int j = 0; j < g->width; j++) {
			switch((int)((double)rand()/r/32768.0)) {
				case 0: {
					g->px[i][j] = 0; //pepper
					break;
				}
				case 1: {
					g->px[i][j] = 255; //salt
					break;
				}
			}
		}
	}
}

//part 6: harmonic mean filtering
void hmnmFtr(Bitmap *g, int z, double Q) {
	//pre-calculate g^Q and g^(Q+1)
	double **n, **d;
	n = new double*[g->height];
	d = new double*[g->height];
	for(int i = 0; i < g->height; i++) {
		n[i] = new double[g->width];
		d[i] = new double[g->width];
		for(int j = 0; j < g->width; j++) {
			n[i][j] = pow((double)g->px[i][j], Q+1.0);
			d[i][j] = pow((double)g->px[i][j], Q);
		}
	}
	//calculate new f
	for(int i = 0; i <= g->height - z; i++){
		double sn = 0, sd = 0;
		for(int k = 0; k < z; k++) for(int j = 0; j < z; j++) {
				sn += n[i+k][j]; sd += d[i+k][j];
			}
		g->px[i + z/2][z/2] = bound(sn / sd);
		for(int j = 0; j < g->width - z; j++){
			for(int k = 0; k < z; k++){ //in a recursive way
				sn -= n[i+k][j]; sn += n[i+k][z+j];
				sd -= d[i+k][j]; sd += d[i+k][z+j];
			}
			g->px[i + z/2][1 + j + z/2] = bound(sn / sd);
		}
	}
}

//part 7: movement blur
void motionBlur(Bitmap *g, double a, double b, double T){
	//initialize
	int n = 2 << (int)ceil(log2(g->height > g->width ? g->height : g->width));
	cmplx **x, **X, **H;
	x = new cmplx*[n];
	X = new cmplx*[n];
	H = new cmplx*[n];
	for(int i = 0; i < n; i++) {
		x[i] = new cmplx[n];
		X[i] = new cmplx[n];
		H[i] = new cmplx[n];
		double u = i > n/2 ? n - i : i;
		for(int j = 0; j < n; j++) {
			double v = j > n/2 ? n - j : j;
			double k, d = M_PI * (a * u + b * v);
			if(d == 0) k = T; else k = T * sin(d) / d;
			H[i][j].re = k * cos(d);
			H[i][j].im = k * -sin(d);
			
			if(i < g->height && j < g->width) x[i][j].re = g->px[i][j];
			else x[i][j].re = 0;
		}
	}
	
	//2D FFT and frequency domain filtering
	FFT fft(n);
	fft.FFT2(x, X);
	for(int i = 0; i < n; i++) {
		for(int j = 0; j < n; j++) {
			//X[i][j] = X[i][j] * H[i][j];
			//*
			X[i][j].re = X[i][j].re * H[i][j].re - X[i][j].im * H[i][j].im;
			X[i][j].im = X[i][j].re * H[i][j].im + X[i][j].im * H[i][j].re;			
			//*/
		}
	}
	//IFFT & recover
	fft.FFT2(X, x);	
	for(int i = 0; i < g->height; i++) {
		for(int j = 0; j < g->width; j++) {
			double m = x[n-i-1][n-j-1].re/n/n;
			g->px[i][j] = bound(m);
		}
	}
}
