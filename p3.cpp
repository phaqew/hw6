#include "hw6.cpp"

int main(){
	BMP256 p("lena.bmp");
	std::cout<<"lena.bmp read"<<std::endl;
	/*
	BMP256 *m;
	char n[] = "_mb0";
	for(int i = 0; i < 5; i++){
		n[3] = '0' + i;
		m = new BMP256(&p);
		motionBlur(m, 0.01*i, 0.01*i, 1);
		m->addNameSuffix(n);
		m->writeFile();
	}
	*/
	BMP256 p_mb(&p);
	motionBlur(&p_mb, 0.05, 0.05, 1);
	p_mb.addNameSuffix("_mb");
	p_mb.writeFile();
	std::cout<<"motion blur added"<<std::endl;
	
}

