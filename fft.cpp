#include<iostream>
#include<fstream>
#include<math.h>
#include<stdlib.h>

class cmplx {
	public:
		double re;
		double im;
		cmplx() {
			re=0;
			im=0;
		}
		double mod() {
			double r = sqrt(re*re+im*im);
			return r;//re>=0 ? r : -r;
		}
		double arg() {
			if(re==0) {
				if(im>0) return M_PI/2;
				if(im<0) return -M_PI/2;
				else return 0;
			}
			return atan(im/re);
		}
		cmplx operator +(cmplx n) {
			cmplx z;
			z.re=re+n.re;
			z.im=im+n.im;
			return z;
		}
		cmplx operator -(cmplx n) {
			cmplx z;
			z.re=re-n.re;
			z.im=im-n.im;
			return z;
		}
		cmplx operator *(cmplx n) {
			cmplx z;
			z.re=re*n.re-im*n.im;
			z.im=re*n.im+im*n.re;
			return z;
		}
		cmplx operator /(double n) {
			cmplx z;
			z.re=re/n;
			z.im=im/n;
			return z;
		}
		static cmplx exp(double ceta) {
			cmplx z;
			z.re=cos(ceta);
			z.im=sin(ceta);
			return z;
		}
		void show() {
			if(re!=0) printf("%f",re);
			if(im>0) printf("+");
			if(im!=0) printf("%fi",im);
			if(re==0&&im==0) printf("0");
			printf("\n");
		}
};

class FFT {
	private:
		int N;
		int *rev;
		cmplx *W;
		void REV(int *rev) {
			rev[0] = 0;
			int H=0, i=1, m=1;
			while((m = m<<1) <= N) {
				H = (H|N)>>1;
				do rev[i] = (~rev[m-i-1])&H;
				while(++i < m);
			}
		}
		void trp(cmplx **x) {
			cmplx t;
			for(int i=1; i<N; i++) {
				for(int j=0; j<i; j++) {
					t = x[i][j];
					x[i][j] = x[j][i];
					x[j][i] = t;
				}
			}
		}
	public:
		FFT(int n = 1024) {
			N = n;
			rev = new int[N];
			REV(rev);
			W = new cmplx[N>>1];
			for(int i=(N>>1)-1; i>=0; i--) W[i]=cmplx::exp(-2.0*M_PI*(double)i/(double)N);
		}
		void FFT1(cmplx *x, cmplx *X) {
			int q=0;
			for(int i=0; i<N; i++) X[i] = x[rev[i]];
			for(int i=N; i>1; i=i>>1) q++;
			for(int n=2; n<=N; n=(n<<1)) {
				q--;
				for(int p=0; p<N; p+=n) {
					int m=p+(n>>1);
					for(int i=(n>>1)-1; i>=0; i--) {
						cmplx t = W[i<<q]*X[m+i];
						X[m+i] = X[p+i] - t;
						X[p+i] = X[p+i] + t;
					}
				}
			}
		}
		void FFT2(cmplx**x, cmplx**X) {
			cmplx**t = new cmplx*[N];
			for(int i=0; i<N; i++) {
				t[i] = new cmplx[N];
				FFT1(x[i],t[i]);
			}
			trp(t);
			for(int i=0; i<N; i++) {
				FFT1(t[i],X[i]);
				delete[] t[i];
			}
			trp(X);
			delete[] t;
		}
		~FFT(){
			delete[] rev;
			delete[] W;
		}
};

