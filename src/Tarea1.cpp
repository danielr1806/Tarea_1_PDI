#include <iostream>
#include <fstream>
using namespace std;
FILE* f;
FILE* f1;
unsigned char **data;
unsigned char *paleta;
unsigned char info[54];
int tam_paleta=0, height=0, width=0, size=0, bpp=0, pad=0;


int Padding(int tam){
	//Calculamos el padding que tiene la imagen
	int p=0;
	while(((tam+p)%4)!=0 && p<4) p++;
    p = tam+p;
    return p;
}

/*void Convertir_24(){
	for(int i=0; i<height; i++){
		unsigned char* data_aux = new unsigned char[Padding(width*3)];
		int k=0;
		for(int j=0; j<size; j++){
			int x = 4*(int)data[i][j];
			data_aux[k] = paleta[x];
			data_aux[k+1] = paleta[x+1];
			data_aux[k+2] = paleta[x+2];
			k += 3;
		}
		data[i] = data_aux;
	}
	info[28] = (unsigned char) 24;
	info[10] = 0x36;
	info[11] = info[12] = info[13] = 0x00;
	size = width*3;
	info[2] = (unsigned char) size*height;
	info[46] = info[47] = info[48] = info[49] = 0x00;
	pad = Padding(size);
}*/

void Guardar(){
	//Guardamos la cabecera, paleta si la tiene, y la data en el archivo de salida
	fwrite(info, sizeof(unsigned char), 54, f1);
	if(bpp != 24){
		fwrite(paleta, sizeof(unsigned char), tam_paleta, f1);
	}
	for(int i = 0; i < height; i++){
	    fwrite(data[i], sizeof(unsigned char), pad, f1);
	} 
}

void negativo(){
	//Modificamos los valores RGB de la imagen para obtener su negativo
	if(bpp!=24){
		for(int i = 0; i < tam_paleta; i += 4){
	        paleta[i] = (unsigned char) (255-(int)paleta[i]);
	        paleta[i+1] = (unsigned char) (255-(int)paleta[i+1]);
	        paleta[i+2] = (unsigned char) (255-(int)paleta[i+2]);
	    }
	}else{	
	    for(int i = 0; i < height; i++){
	    	for(int j = 0; j < width*3; j++){
	    		data[i][j] = (unsigned char) (255-(int)data[i][j]);
	    	}
	    } 
	}	
}

void espejo_horizontal(){
	
    for(int i = 0; i < height ; i++){
    	unsigned char* data_aux = new unsigned char[size];
    	if(bpp!=24){
	    	for(int j = 0; j < size; j += 4){
	    		data_aux[j] = data[i][size-j-2];
	    		data_aux[j+1] = data[i][size-j-3];
	    		data_aux[j+2] = data[i][size-j-4];
	    		data_aux[j+3] = data[i][size-j-1];
	    	}
	    }else{
	    	for(int j = 0; j < size; j += 3){
	    		data_aux[j] = data[i][pad-j-3];
	        	data_aux[j+1] = data[i][pad-j-2];
	        	data_aux[j+2] = data[i][pad-j-1];
	    	}
	    }
	    data[i] = data_aux;
    }
}

void espejo_vertical(){
	//Invertimos las filas de la matriz, quedando las primeras filas al final y las ultimas de primero
    for(int i = 0; i < height/2 ; i++){
    	swap(data[i],data[height-i-1]);
    }
}

void rotar_derecha(){
	
	for(int i=18; i<22; i++){
		swap(info[i],info[i+4]);
	}
    
    int size_h = height*size/width;
	int pad_h = Padding(size_h);
	
	unsigned char** data_aux = new unsigned char* [width];
	for(int i = 0; i < width; i ++){
		int k=0;
		data_aux[i] = new unsigned char[pad_h];
		for(int j = 0; j < height; j++){
	        data_aux[i][k] = data[j][size-(i*3)-3];
	     	data_aux[i][k+1] = data[j][size-(i*3)-2];
	        data_aux[i][k+2] = data[j][size-(i*3)-1];
	        k += 3;
		}
		if(pad_h > size_h){
			while(k<pad_h){
				data_aux[i][k]=0;
				k++;
			}
		}
	}
	data = data_aux;
	swap(height,width);
	size = size_h;
	pad = pad_h;
}

void rotar_izquierda(){
	
	for(int i=18; i<22; i++){
		swap(info[i],info[i+4]);
	}
    
    int size_h = height*size/width;
	int pad_h = Padding(size_h);
	
	unsigned char** data_aux = new unsigned char* [width];
	for(int i = 0; i < width; i ++){
		int k=0;
		data_aux[i] = new unsigned char[pad_h];
		for(int j = height; j > 0; j--){
	        data_aux[i][k] = data[j-1][i*3];
	     	data_aux[i][k+1] = data[j-1][i*3+1];
	        data_aux[i][k+2] = data[j-1][i*3+2];
	        k += 3;
		}
		if(pad_h > size_h){
			while(k<pad_h){
				data_aux[i][k]=0;
				k++;
			}
		}
	}
	data = data_aux;
	swap(height,width);
	size = size_h;
	pad = pad_h;
}

void ReadBMP(const char* filename){
	
    f = fopen(filename, "rb");
    if(f == NULL)
        throw "Argument Exception";
    
    //Se lee la cabecera de 54 bits
    fread(info, sizeof(unsigned char), 54, f); 
    
    //Guarda informacion relevante de la cabecera, como alto, ancho y profundidad en bits
    width = *(int*)&info[18];
    height = *(int*)&info[22];
    bpp = *(int*)&info[28];
    
    //Se calcula el size de la imagen y, en caso de que tenga paleta, se guarda su tamaño
    if(bpp==1){
        size = width/8;
        tam_paleta = 8;
    }
    if(bpp==4){
        tam_paleta = 64;
        size = width/2;
    }
    if(bpp==8){
        tam_paleta = 256*bpp;
        size = width;
    }
    if(bpp==24){
        size = 3*width;
    }else{
    	//Se guarda la paleta
        paleta = new unsigned char[tam_paleta];
        fread(paleta, sizeof(unsigned char), tam_paleta, f);
    }
    
    //Llamamos a Padding para calcular el relleno de la imagen
	pad = Padding(size);
    
    //Leemos el resto de la data de la imagen
    data = new unsigned char* [height];
    for(int i = 0; i < height; i++){
    	data[i] = new unsigned char[pad];
    	fread(data[i], sizeof(unsigned char), pad, f);
    }
    //if(bpp != 24) Convertir_24();
    
    fclose(f);
}

int main(){
	string imagen;
	int n=0;
	do{
		cout<<"Introduzca nombre del archivo"<<endl;
		cin>>imagen;
		ReadBMP(imagen.c_str());
		do{
			cout<<endl<<"Menu"<<endl<<"1.Negativo de la imagen"<<endl<<"2. Espejo horizontal"<<endl<<"3. Espejo vertical"<<endl;
			cout<<"4. Rotar 90 CW"<<endl<<"5. Rotar 90 CCW"<<endl<<"6. Rotar 180"<<endl<<"7. Rotar 270 CW"<<endl<<"8. Rotar 270 CCW"<<endl;
			cout<<"9. Cargar nueva imagen"<<endl;
			cin>>n;
			if(n>0 && n<9){
			
				f1 = fopen("salida.bmp", "wb");
			    if(f1 == NULL)
			        throw "Argument Exception";
			
				switch(n){
					
					case 1:
						//////negativo//////
						negativo();
					break;
					
					case 2:
						/////espejo horizontal/////
					    espejo_horizontal();
						
					break;
				    
				    case 3:
					    /////espejo vertical/////
						espejo_vertical();
					break;
			    
			    	case 4:
				    	/////Rotar 90 CW/////
				    	rotar_derecha();
					break;
					
					case 5:
						/////Rotar 90 CCW/////
						rotar_izquierda();
					break;
					
					case 6:
						/////Rotar 180/////
						rotar_derecha();
						rotar_derecha();
					break;
					
					case 7:
						/////Rotar 270 CW/////
						rotar_izquierda();
					break;
					
					case 8:
						/////Rotar 270 CCW/////
						rotar_derecha();
					break;
				}
				Guardar();
			    fclose(f1);
			}
		}while(n>0 && n<9);	
		cout<<endl;
	}while(n==9);
	return 0;
}
