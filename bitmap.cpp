#include<fstream>
#include<iostream>
#include<windows.h>
#define DEBUG 0 //set true to show debug information
class Bitmap {
	public:
		char name[50];
		int height;
		int width;
		BYTE **px;
		~Bitmap() {
			for(int i = 0; i < height; i++) delete[] px[i];
			delete[] px;
			if(DEBUG) std::cout << "Bitmap destructed." << std::endl;
		}
		void setName(const char *n, const char *s = NULL){
			if(s == NULL){
				for(int i = 0; i < 50; i++){
					name[i] = n[i];
					if(name[i] == '\0') {
						while(i < 49) name[++i] = 0;
						break;
					}
				}
			} else {
				for(int i = 0; i < 50; i++){
					name[i] = n[i];
					if(name[i] == '.'){
						for(int j = 0; s[j]; ) name[++i] = s[j++];
						while(i < 49) name[++i] = 0;
						break;
					}
					if(n[i+1] == '\0'){
						name[++i] = '.';
						for(int j = 0; s[j]; ) name[++i] = s[j++];
						while(i < 49) name[++i] = 0;
						break;
					}
				}
			}
		}
		void addNameSuffix(const char *n){
			int i, c;
			for(c = 0; n[c] != '\0'; c++) ;
			for(i = 0; name[i] != '\0'; i++) ;
			do name[c+i] = name[i]; while(name[i--] != '.');
			for(i += c; c > 0; i--) name[i] = n[--c];
		}
};

class PGM5 : public Bitmap {
	public:
		int maxg;
		PGM5(const char* file) {
			for(int i = 0; ; i++) if(file[i] == '\0') {do name[i] = file[i]; while(i--); break;}
			std::ifstream in(file, std::ios::in | std::ios::binary);
			if (!in) {
				std::cout << "Read file error!" << std::endl;
				throw 0;
			}
			if(DEBUG) std::cout << "Reading from file " << file << std::endl;
			//filehead
			char tc[3];
			in.read(tc, 3);
			if(tc[0] != 'P' || tc[1] != '5' || tc[2] != '\n') {
				std::cout << "Not a P5 pgm file!" << std::endl;
				throw 0;
			}
			in.read(tc, 1);
			if(tc[0] == '#') while(tc[0] != '\n') in.read(tc, 1);
			else in.seekg(3, std::ios_base::beg);
			in >> width >> height >> maxg;
			in.read(tc, 1);
			//read pxs
			px = new BYTE*[height];
			for (int i = height - 1; i >= 0; i--) {
				px[i] = new BYTE[width];
				in.read((char*)px[i], width);
			}
			if(DEBUG) std::cout << "Reading finished." << std::endl;
			in.close();
		}
		void writeFile(const char* file) {
			std::ofstream out(file, std::ios::out | std::ios::binary);
			if (!out) {
				std::cout << "Write file error!" << std::endl;
				throw 0;
			}
			if(DEBUG) std::cout << "Writing to file " << file << std::endl;
			out << "P5\n" << height << " " << width << "\n" << maxg << "\n";
			for (int i = 0; i < height; i++) out.write((char*)px[i], width);
			if(DEBUG) std::cout << "Writing finished." << std::endl;
			out.close();
		}
};

class TIF : public Bitmap {
	public:
		struct DE {
			unsigned short tag;
			short typ;
			int amt;
			int loc;
		};
		TIF(const char *file) {
			for(int i = 0; ; i++) if(file[i] == '\0') {do name[i] = file[i]; while(i--); break;}
			std::ifstream in(file, std::ios::in | std::ios::binary);
			if(DEBUG) std::cout << "Reading from file " << file << std::endl;
			int ifdp;
			in.read((char*)&ifdp, 4);
			if(ifdp != 2771273) throw 0; //002A4949
			in.read((char*)&ifdp, 4);
			//assume one file only contain one pic
			//while(ifdp != 0) {
			in.seekg(ifdp, std::ios_base::beg);
			short a;
			in.read((char*)&a, 2);
			DE de;
			int p;
			bool ex = false;
			while(a--) {
				in.read((char*)&de, sizeof(DE));
				if(DEBUG) std::cout << de.tag << "\t" << de.typ << "\t" << de.amt << "\t" << de.loc << "\n";
				switch(de.tag) {
					case 256: //height
						height = de.loc;
						break;
					case 257: //width
						width = de.loc;
						break;
					case 273: //data blocks location
						p = de.loc;
						break;
					case 277:
						if(de.loc == 2) ex = true;
						break;
					case 338:
						if(de.loc == 1) ex = true;
						break;
					default: //ignore other information
						break;
				}
			}
			in.read((char*)&ifdp, 4);
			//}
			in.seekg(p, std::ios_base::beg);
			in.read((char*)&p, 4); //get the beginning of pixel data
			in.seekg(p, std::ios_base::beg);
			//read pxs
			px = new BYTE*[height];
			for (int i = height - 1; i >= 0; i--) {
				px[i] = new BYTE[width];
				for(int j = 0; j < width; j++){
					px[i][j] = in.get();
					if(ex) in.get();
				}
			}
			in.close();
		}
};

class BMP256 : public Bitmap {
	public:
		BITMAPFILEHEADER fileHead;
		BITMAPINFOHEADER bmpInfoHead;
		struct Palette {
			struct Color {
				BYTE B;
				BYTE G;
				BYTE R;
				BYTE A;
			};
			Color color[256];
			int count;
			void read(std::ifstream *in, int c) {
				count = (c == 0 ? 256 : c);
				in->read((char*)color, 4 * count);
			}
			void write(std::ofstream *out) {
				out->write((char*)color, 4 * count);
			}
		};
		Palette palette;

		BMP256(const char* file) {
			for(int i = 0; ; i++) if(file[i] == '\0') {do name[i] = file[i]; while(i--); break;}
			//filestream
			std::ifstream inBmp(file, std::ios::in | std::ios::binary);
			if (!inBmp) {
				std::cout << "Read file error!" << std::endl;
				throw 0;
			}
			if(DEBUG) std::cout << "Reading from file " << file << std::endl;
			//filehead
			inBmp.read((char*)&fileHead, sizeof(fileHead));
			inBmp.read((char*)&bmpInfoHead, sizeof(bmpInfoHead));
			if (bmpInfoHead.biBitCount != 8) {
				std::cout << "Not a 256 color bitmap!" << std::endl;
				throw 0;
			}
			height = bmpInfoHead.biHeight;
			width = bmpInfoHead.biWidth;
			palette.read(&inBmp, bmpInfoHead.biClrUsed);
			//read pxs
			px = new BYTE*[height];
			for (int i = 0; i < height; i++) px[i] = new BYTE[width];
			int dataSizePerLine = ((width + 7) / 8) * 8;
			for (int i = 0; i < height; i++) {
				inBmp.seekg(fileHead.bfOffBits + i * dataSizePerLine, std::ios_base::beg);
				for (int j = 0; j < width; j++) px[i][j] = inBmp.get();
			}
			if(DEBUG) std::cout << "Reading finished." << std::endl;
			inBmp.close();
		}

		BMP256(int row, int column) {
			width = column;
			height = row;
			int dataSizePerLine = ((column + 7) / 8) * 8;
			int fileSize = row * dataSizePerLine + 1078;
			fileHead.bfType = 19778;//short		"BM"
			fileHead.bfSize = fileSize;//long
			fileHead.bfReserved1 = 0;//short
			fileHead.bfReserved2 = 0;//short
			fileHead.bfOffBits = 1078;//long	0436h
			bmpInfoHead.biSize = sizeof(bmpInfoHead);//long
			bmpInfoHead.biWidth = width;//long
			bmpInfoHead.biHeight = height;//long
			bmpInfoHead.biPlanes = 1;//short
			bmpInfoHead.biBitCount = 8;//short
			bmpInfoHead.biCompression = 0;//long
			bmpInfoHead.biSizeImage = fileSize - 1078;//long
			bmpInfoHead.biXPelsPerMeter = 0;//long
			bmpInfoHead.biYPelsPerMeter = 0;//long
			bmpInfoHead.biClrUsed = 0;//long
			bmpInfoHead.biClrImportant = 0;//long
			//initial a gray palette
			palette.count = 256;
			for(int i = 0; i < 256; i++) {
				palette.color[i].B = i;
				palette.color[i].G = i;
				palette.color[i].R = i;
				palette.color[i].A = 0;
			}
			//fill blank image with 0
			px = new BYTE*[height];
			for (int i = 0; i < height; i++) {
				px[i] = new BYTE[width];
				for (int j = 0; j < width; j++)
					px[i][j] = 0;
			}
		}

		BMP256(BMP256* b) {
			this->width = b->width;
			this->height = b->height;
			this->fileHead = b->fileHead;
			this->bmpInfoHead = b->bmpInfoHead;
			this->palette.count = b->palette.count;
			for(int i = 0; i < palette.count; i++)
				this->palette.color[i] = b->palette.color[i];
			px = new BYTE*[height];
			for (int i = 0; i < height; i++) {
				px[i] = new BYTE[width];
				for (int j = 0; j < width; j++)
					this->px[i][j] = b->px[i][j];
			}
			for(int i = 0; this->name[i] = b->name[i]; i++) ;
		}

		void writeFile(int xd = 0, int yd = 0, const char* path = NULL) {
			int dataSizePerLine = ((width + 7) / 8) * 8;
			int fileSize = height * dataSizePerLine + 1078;
			std::ofstream outBmp;
			if(path == NULL) outBmp.open(name, std::ios::out | std::ios::binary);
			else outBmp.open(path, std::ios::out | std::ios::binary);
			if (!outBmp) {
				std::cout << "Write file error!" << std::endl;
				throw 0;
			}

			if(DEBUG) std::cout << "Writing file " << name << std::endl;
			outBmp.write((char*)&fileHead, sizeof(fileHead));
			outBmp.write((char*)&bmpInfoHead, sizeof(bmpInfoHead));
			palette.write(&outBmp);

			BYTE* buffer = new BYTE[dataSizePerLine];
			for (int i = yd; i < height; i++) {
				int k = 0;
				for (int j = xd; j < width; j++) buffer[k++] = px[i][j];
				for (int j = 0; j < xd; j++) buffer[k++] = px[i][j];
				outBmp.write((char*)buffer, dataSizePerLine);
			}
			for (int i = 0; i < yd; i++) {
				int k = 0;
				for (int j = xd; j < width; j++) buffer[k++] = px[i][j];
				for (int j = 0; j < xd; j++) buffer[k++] = px[i][j];
				outBmp.write((char*)buffer, dataSizePerLine);
			}
			delete[] buffer;
			if(DEBUG) std::cout << "Writing finished." << std::endl;
			outBmp.close();
		}
};
