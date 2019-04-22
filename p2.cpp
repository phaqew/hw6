#include "hw6.cpp"

int main(){
	BMP256 p("lena.bmp");
	std::cout<<"lena.bmp read"<<std::endl;
	BMP256 p_sp(&p);
	addSaltPepperNoise(&p_sp, 0.1);
	p_sp.addNameSuffix("_sp");
	std::cout<<"salt and pepper noise added"<<std::endl;
	BMP256 p_sp_hmn2f(&p_sp);
	hmnmFtr(&p_sp_hmn2f, 5, -2);
	p_sp_hmn2f.addNameSuffix("_hmn2f");
	p_sp_hmn2f.writeFile();
	std::cout<<"harmonic mean filtering (z = 5, Q = -2) processed"<<std::endl;
	BMP256 p_sp_hmn1f(&p_sp);
	hmnmFtr(&p_sp_hmn1f, 5, -1);
	p_sp_hmn1f.addNameSuffix("_hmn1f");
	p_sp_hmn1f.writeFile();
	std::cout<<"harmonic mean filtering (z = 5, Q = -1) processed"<<std::endl;
	BMP256 p_sp_hm0f(&p_sp);
	hmnmFtr(&p_sp_hm0f, 5, 0);
	p_sp_hm0f.addNameSuffix("_hm0f");
	p_sp_hm0f.writeFile();
	std::cout<<"harmonic mean filtering (z = 5, Q = 0) processed"<<std::endl;
	BMP256 p_sp_hmp1f(&p_sp);
	hmnmFtr(&p_sp_hmp1f, 5, 1);
	p_sp_hmp1f.addNameSuffix("_hmp1f");
	p_sp_hmp1f.writeFile();
	std::cout<<"harmonic mean filtering (z = 5, Q = +1) processed"<<std::endl;
	BMP256 p_sp_hmp2f(&p_sp);
	hmnmFtr(&p_sp_hmp2f, 5, 2);
	p_sp_hmp2f.addNameSuffix("_hmp2f");
	p_sp_hmp2f.writeFile();
	std::cout<<"harmonic mean filtering (z = 5, Q = +2) processed"<<std::endl;
}

