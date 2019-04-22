#include "hw6.cpp"

int main(){
	BMP256 p("lena.bmp");
	std::cout<<"lena.bmp read"<<std::endl;
	/*
	BMP256 *t;
	t = new BMP256(&p);
	addGaussianNoise(t, 10, 10);
	t->addNameSuffix("_gn11");
	t->writeFile();
	t = new BMP256(&p);
	addGaussianNoise(t, 10, 20);
	t->addNameSuffix("_gn12");
	t->writeFile();
	t = new BMP256(&p);
	addGaussianNoise(t, -10, 10);
	t->addNameSuffix("_gn21");
	t->writeFile();
	t = new BMP256(&p);
	addGaussianNoise(t, -10, 20);
	t->addNameSuffix("_gn22");
	t->writeFile();
	*/
	BMP256 p_gn(&p);
	addGaussianNoise(&p_gn, 10, 10);
	p_gn.addNameSuffix("_gn");
	p_gn.writeFile();
	std::cout<<"gaussian noise added"<<std::endl;
	
	BMP256 p_gn_smf(&p_gn);
	sptMidFtr(&p_gn_smf, 5);
	p_gn_smf.addNameSuffix("_smf");
	p_gn_smf.writeFile();
	std::cout<<"spatial midian filtering processed"<<std::endl;
	
	BMP256 p_gn_sgf(&p_gn);
	sptGsnFtr(&p_gn_sgf, 5);
	p_gn_sgf.addNameSuffix("_sgf");
	p_gn_sgf.writeFile();
	std::cout<<"spatial gaussian filtering processed"<<std::endl;
	
	BMP256 p_gn_fbf(&p_gn);
	frqBtwFtr(&p_gn_fbf, 120, 2);
	p_gn_fbf.addNameSuffix("_fbf");
	p_gn_fbf.writeFile();
	std::cout<<"frequency butterworth filtering processed"<<std::endl;
	
}

