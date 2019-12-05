#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include "puff.h"
#include "puff.c"

//extensive use of https://www.w3.org/TR/2003/REC-PNG-20031110/

using namespace std;

struct file{
    string file_name;
    long long IHDR_length;
    long long width = 0;
    long long height = 0;
    long long depth;
    long long colour;
    long long compress;
    long long filter;
    long long inter;
    long long IHDR_CRC = 0;
    long long pHYs_length;
    long long pHYs_CRC = 0;
    long long cHRM_length;
    long long cHRM_CRC = 0;
    long long IdAT_length;
    long long IdAT_first;
    long long IdAT_CRC = 0;
    long long IEND_length;
    long long IEND_CRC = 0;
    long long IEND_first = 0;
};//the struct which containts all my data 

file png;//instantiation of the struct 
int check = 0;
unsigned char *stream;//loading of the entire file
unsigned long** newdata;//the 2d array for the decompressed data
unsigned long* newtype;//the array which holds the filter type for each row of data

void sign();//method to find the signature and confirm that the file is a png
void header();//method to get information about the file
void phys();//method to find the phys length and crc
void chr();//method to find the chr length and crc
void data();//method to find the data
void process();//method to use the puff() and decompress the data
void unfilter();//method to apply all the unfiltering algorithms on the data
void fix(int i, int j);//method to adjust the pixel data to be within 255
unsigned long paethPredictor(unsigned long a, unsigned long b, unsigned long c);//method used for filtration type 4
void print5();//method to print the 5x5 corner
void end();//method to find the end and its crc
void toRed();//method to create red variation of the image file
void toGreen();//method for green variation
void toBlue();//method for blue variation
void out(unsigned long** tempdata, int colour);//method to create and write to a file

int main(){
    while(check == 0 || check == 1) {//used to check the validity of the file when it is first loaded
        cout << "\nImage Processing Software\nSpecify the name of a PNG file that you would like to process.\n>";
        cin >> png.file_name;
        png.file_name = png.file_name + ".png";
        sign();
        if (check == 0) {
            cout << "The uploaded file is not a png";
        } else if (check == 1) {
            cout << "This file does not exist";
        }
    }
    header();
    phys();
    chr();
    data();
    process();
    unfilter();
    print5();
    end();
    toBlue();
    toRed();
    toGreen();
    cout << "\nRed, Blue, and Green versions of " << png.file_name << " made.(Not really because my program outputs 3 images with errors)";
}

void sign() {
    streampos size;
    stringstream temper;
    string tempe;
    ifstream file(png.file_name, ios::in | ios::binary | ios::ate);//opens file in binary format
    if (file.is_open()){//checks if file is valid
        size = file.tellg();
        stream = (new unsigned char[size]);
        cout << "Loading file " << png.file_name << "\n";
        file.seekg(0, ios::beg);
        file.read(reinterpret_cast<char *>(stream), size);//reads the whole file into array
        for (int i = 0; i < 8; i++) {//checks signature
            temper << (long long)(stream[i]);
        }
        temper >> tempe;
        if(tempe != "13780787113102610"){//checks if even if file is valid, is it a png using signature
            check = 0;
            return;
        }
    }
    else{//returns for invalid file
        check = 1;
        return;
    }
    check = 2;
    cout << "\nSuccessful Loading of file " << png.file_name << "\n";
    file.close();
}

void header(){
    int checker = 0, i = 0;
    while(checker == 0){//checks until it finds the 4 bytes which signify header
        if(((long long)stream[i] == 73) && ((long long)stream[i + 1] == 72) && ((long long)stream[i + 2] == 68) && ((long long)stream[i + 3] == 82)){
            for(int j = 0; j < 4; j++){//gets header length
                png.IHDR_length = (png.IHDR_length << 8) | (long long)stream[i - (4 - j)];
            }
            for(int j = 0; j < 4; j++){//width of file
                png.width = (png.width << 8) | (long long)stream[i + 4 + j];
            }
            for(int j = 0; j < 4; j++){//height of file
                png.height = (png.height << 8) | (long long)stream[i + 8 + j];
            }
            png.depth = (long long)stream[i + 12];
            png.colour = (long long)stream[i + 13];
            png.compress = (long long)stream[i + 14];
            png.filter = (long long)stream[i + 15];
            png.inter = (long long)stream[i + 16];
            for(int j = 0; j < 4; j++){
                png.IHDR_CRC = (png.IHDR_CRC << 8) | (long long)stream[i + png.IHDR_length + 4 + j];
            }
            checker = 1;
        }
        i++;
    }
    cout << "\nIHDR  " << png.IHDR_length << "  " << png.IHDR_CRC;
    cout << "\nWidth:\t\t" << png.width;
    cout << "\nheight:\t\t" << png.height;
    cout << "\nbitdepth:\t" << png.depth;
    cout << "\ncolortype:\t" << png.colour;
    cout << "\ncomp method:\t" << png.compress;
    cout << "\nfilt method:\t" << png.filter;
    cout << "\nintl method:\t" << png.inter;
}

void phys(){
    int checker = 0, i = 0;
    while(checker == 0){//checks until it finds bytes which signify the phys
        if(((long long)stream[i] == 112) && ((long long)stream[i + 1] == 72) && ((long long)stream[i + 2] == 89) && ((long long)stream[i + 3] == 115)){
            for(int j = 0; j < 4; j++){
                png.pHYs_length = (png.pHYs_length << 8) | (long long)stream[i - (4 - j)];
            }
            for(int j = 0; j < 4; j++){
                png.pHYs_CRC = (png.pHYs_CRC << 8) | (long long)stream[i + png.pHYs_length + 4 + j];
            }
            checker = 1;
        }
        i++;
    }
    cout << "\n\npHYs  " << png.pHYs_length << "  " << png.pHYs_CRC;
}

void chr(){
    int checker = 0, i = 0;
    while(checker == 0){//checks until it finds bytes which signify the chrm
        if(((long long)stream[i] == 99) && ((long long)stream[i + 1] == 72) && ((long long)stream[i + 2] == 82) && ((long long)stream[i + 3] == 77)){
            for(int j = 0; j < 4; j++){
                png.cHRM_length = (png.cHRM_length << 8) | (long long)stream[i - (4 - j)];
            }
            for(int j = 0; j < 4; j++){
                png.cHRM_CRC = (png.cHRM_CRC << 8) | (long long)stream[i + png.cHRM_length + 4 + j];
            }
            checker = 1;
        }
        i++;
    }
    cout << "\n\ncHRM  " << png.cHRM_length << "  " << png.cHRM_CRC;
}

void data(){
    int checker = 0, i = 0;
    while(checker == 0){//checks until it finds bytes which signify the idat, stores where the data in idat begins for later
        if(((long long)stream[i] == 73) && ((long long)stream[i + 1] == 68) && ((long long)stream[i + 2] == 65) && ((long long)stream[i + 3] == 84)){
            for(int j = 0; j < 4; j++){
                png.IdAT_length = (png.IdAT_length << 8) | (long long)stream[i - (4 - j)];
            }
            for(int j = 0; j < 4; j++){
                png.IdAT_CRC = (png.IdAT_CRC << 8) | (long long)stream[i + png.IdAT_length + 4 + j];
            }
            png.IdAT_first = i + 4;
            checker = 1;
        }
        i++;
    }
    cout << "\n\nIDAT  " << png.IdAT_length << "  " << png.IdAT_CRC;
}

void end(){
    int checker = 0, i = 0;
    while(checker == 0){//checks until it finds the bytes which signify the data for the iend
        if(((long long)stream[i] == 73) && ((long long)stream[i + 1] == 69) && ((long long)stream[i + 2] == 78) && ((long long)stream[i + 3] == 68)){
            for(int j = 0; j < 4; j++){
                png.IEND_length = (png.IEND_length << 8) | (long long)stream[i - (4 - j)];
            }
            for(int j = 0; j < 4; j++){
                png.IEND_CRC = (png.IEND_CRC << 8) | (long long)stream[i + png.IEND_length + 4 + j];
            }
            checker = 1;
            png.IEND_first = i;
        }
        i++;
    }
    cout << "\n\nIEND  " << png.IEND_length << "  " << png.IEND_CRC;
}

void process(){
    //declaration of variables to be used with puff
    unsigned long len = png.IdAT_length - 6;
    unsigned char* dest = NIL;
    unsigned long destlen = 0;
    unsigned char* source = new unsigned char[len];
    for(int i = 0; i < len; i++){//getting useful data from the idat to decompress
        source[i] = stream[png.IdAT_first + 2 + i];
    }
    cout << "\nputon " << (long long)source[0];
    int out = puff(dest, &destlen, source, &len);//puff once to find the length of the uncompressed data
    dest = new unsigned char[destlen];
    puff(dest, &destlen,source,&len);//puff a second time to load the uncompressed data into the array dest
    if(out == 0) {//checks if decompression was successful
        cout << "\npuff() succeeded uncompressing " << destlen << " bytes";
    }
    int counter = 0;
    newdata = new unsigned long*[png.height];//declaration of dynamically allocated 2d array
    for(int i = 0; i < png.height; i++){
        newdata[i] = new unsigned long[png.width*3];
    }
    newtype = new unsigned long[png.height];
    for(int i = 0; i < png.height; i++){//loading the data from dest into the 2d array
        newtype[i] = (long long)dest[counter];//stores the filter type for each row
        counter++;
        for(int j = 0; j < png.width*3; j++){
            newdata[i][j] = (long long)dest[counter];//stores the data
            counter++;
        }
    }
}

void unfilter(){
    int type;//uses the newtype array to check which filtration type to apply on the row of data and then unfilters the whole 2d array
    for(int i = 0; i < png.height; i++){
        type = newtype[i];
        //filtration types gotten from png specification
        if(type == 1){
            for(int j = 3; j < png.width*3; j++){
                newdata[i][j] = newdata[i][j] + newdata[i][j-3];
                fix(i,j);
            }
        }
        else if(type == 2){
            if(i != 0){
                for(int j = 0; j < png.width*3; j++){
                    newdata[i][j] = newdata[i][j] + newdata[i-1][j];
                    fix(i,j);
                }
            }
        }
        else if(type == 3){
            if(i == 0){
                for(int j = 0; j < png.width*3; j++){
                    newdata[i][j] = newdata[i][j] + floor((newdata[i][j-3])/2);
                    fix(i,j);
                }
            }
            else{
                for(int j = 0; j < 3; j++){
                    newdata[i][j] = newdata[i][j] + floor((newdata[i-1][j])/2);
                    fix(i,j);
                }
                for(int j = 3; j < png.width*3; j++){
                    newdata[i][j] = newdata[i][j] + floor(((newdata[i-1][j])+(newdata[i][j-3]))/2);
                    fix(i,j);
                }
            }
        }
        else if(type == 4){
            if(i == 0){
                for(int j = 0; j < 3; j++){
                    newdata[i][j] = newdata[i][j] + paethPredictor(0,0,0);
                    fix(i,j);
                }
                for(int j = 3; j < png.width*3; j++){
                    newdata[i][j] = newdata[i][j] + paethPredictor(newdata[i][j-3],0,0);
                    fix(i,j);
                }
            }
            else{
                for(int j = 0; j < 3; j++){
                    newdata[i][j] = newdata[i][j] + paethPredictor(0,newdata[i-1][j],0);
                    fix(i,j);
                }
                for(int j = 3; j < png.width*3; j++){
                    newdata[i][j] = newdata[i][j] + paethPredictor(newdata[i][j-3],newdata[i-1][j],newdata[i-1][j-3]);
                    fix(i,j);
                }
            }
        }

    }
}

void fix(int i, int j){//makes sure pixel value < 256
    if(newdata[i][j] > 255){
        newdata[i][j] = newdata[i][j] - 256;
    }
}

unsigned long paethPredictor(unsigned long a, unsigned long b, unsigned long c){//algorithm for filtration type 4 from png specification
    int pr;
    int p = a + b - c;
    int pa = p - a;
    if(pa < 0){pa = pa*-1;}
    int pb = p - b;
    if(pb < 0){pb = pb*-1;}
    int pc = p - c;
    if(pc < 0){pc = pc*-1;}
    if((pa <= pb) && (pa <= pc)){
        pr = a;
    }
    else if(pb <= pc){
        pr = b;
    }
    else{
        pr = c;
    }
    return pr;
}

void print5(){//prints 5x5 top left corner
    cout << "\nCorner 5x5:\n";
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 15; j++){
            if((j % 3) == 0) {
                cout << "    ";
            }
            cout << " " << newdata[i][j];
        }
        cout << "\n";
    }
}

void toRed(){//creates a temporary 2d array which has all the values of the normal array but with blue and green to 0
    unsigned long** tempdata = newdata;
    for(int i = 0; i < png.height; i++){
        for(int j = 1; j < png.width*3; j=j+3){
            tempdata[i][j] = 0;
        }
        for(int j = 2; j < png.width*3; j=j+3){
            tempdata[i][j] = 0;
        }
    }
    out(tempdata,1);
}

void toGreen(){//creates a temporary 2d array which has all the values of the normal array but with blue and red to 0
    unsigned long** tempdata = newdata;
    for(int i = 0; i < png.height; i++){
        for(int j = 0; j < png.width*3; j=j+3){
            tempdata[i][j] = 0;
        }
        for(int j = 2; j < png.width*3; j=j+3){
            tempdata[i][j] = 0;
        }
    }
    out(tempdata,2);
}

void toBlue(){//creates a temporary 2d array which has all the values of the normal array but with red and green to 0
    unsigned long** tempdata = newdata;
    for(int i = 0; i < png.height; i++){
        for(int j = 0; j < png.width*3; j=j+3){
            tempdata[i][j] = 0;
        }
        for(int j = 1; j < png.width*3; j=j+3){
            tempdata[i][j] = 0;
        }
    }
    out(tempdata,3);
}

void out(unsigned long** tempdata, int colour){//writes new file
    unsigned char *outdata = new unsigned char[png.width*3*png.height];//output data stream
    int counter = 0;
    for(int i = 0; i < png.height; i++){
        for(int j = 0; j < png.width*3;j++){
            outdata[counter] = tempdata[i][j];
            counter++;
        }
    }
    int checker = 0;
    ofstream tempf;
    if(colour == 1) {//checks which file
        tempf.open("redbrainbow.png", ios::out | ios::app | ios::binary);
    }
    else if(colour == 2) {
        tempf.open("greenbrainbow.png", ios::out | ios::app | ios::binary);
    }
    else if(colour == 3) {
        tempf.open("bluebrainbow.png", ios::out | ios::app | ios::binary);
    }
    for(int i = 0; i < png.IdAT_first; i++){//outputs unchanged beginning of file
        tempf << stream[i];
    }
    tempf << (unsigned char)(120);//zlib first byte
    tempf << (unsigned char)(1);//zlib second byte
    counter = 0;
    while(checker == 0){//the data in IDAT uncompressed sent in chunks
        //header for each chunk
        tempf << (unsigned char)(0);
        tempf << (unsigned char)(255);
        tempf << (unsigned char)(255);
        tempf << (unsigned char)(0);
        tempf << (unsigned char)(0);
        //the actual data being sent in the maximum sized chunks below
        for(int i = 0; i < 65535; i++){
            if(counter > png.width*3*png.height-1){
                checker = 1;
                tempf << 0;
            }
            else {
                tempf << outdata[counter];
                counter++;
            }
        }
        /*if((png.IdAT_length - counter) < 65535){
            checker = 1;
            for(int i = 0; i < (png.IdAT_length - counter); i++){
                tempf << outdata[counter];
                counter++;
            }
            for(int i = 0; i < 65535 - (png.IdAT_length - counter); i++){
                tempf << 0;
            }
        }
        else{
            for (int i = 0; i < 65535; i++) {
                tempf << outdata[counter];
                counter++;
            }
        }*/
    }
    //check sum, crc and length of iend
    for(int i = 0; i < 12; i++){
        tempf << (unsigned char)(0);
    }
    //iend and its crc
    for(int i = 0; i < 8; i++){
        tempf << stream[png.IEND_first+i];
    }
}


