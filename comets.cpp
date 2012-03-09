/* comets.cpp - 03.09.2005 */
/* Kiril comets */

#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

enum part {comet, head, tail};

class Paper {
friend class Rect;     
public:
  Paper();
  ~Paper();
  Paper(const Paper&);
  Paper::Paper(int, int);
  Paper &operator=(const Paper&);
  void read_bin(istream&);
  void write_bin(ostream&) const;
  
  unsigned char &pixel(int x, int y)
     { return text[y*len1 + x]; }
  unsigned char pixel(int x, int y) const
     { return text[y*len1 + x]; }     
  
  void set_items(const Paper&);
  void draw_items(int, int);
  void draw_blobs(string);
  
  /********* Kiril ***************************/    
  void histo(string);
  void rectangles();
  void reduce();
  void calculate(string) const;
private:
   string s1, s2;     
   int len1, len2;
   long len;
   int depth;
   unsigned char* text;  // the image
   vector<unsigned int> his; 
   vector<Rect> items;
   int MMAXval, MAXval, MMINval, MINval, comet_val;    
   
   void read_header(istream&);
   void write_header(ostream&) const;
};

class Rect {
public:
     Rect();
     Rect(int, int);
     void setXmax(int x) { Xmax = x; }
     void setXmin(int x) { Xmin = x; } 
     void setYmax(int y) { Ymax = y; }
     void setYmin(int y) { Ymin = y; } 
     int getXmax() const { return Xmax; }
     int getXmin() const { return Xmin; }
     int getYmin() const { return Ymin; }
     int getYmax() const { return Ymax; }
     int getXcen() const { return Xcen; }  
     int getYcen() const { return Ycen; }
     
     void draw(Paper&, int, int) const;
     void draw_item(const Paper&, Paper&) const;
     bool section(const Rect&) const;
         
     bool is_inside(int, int) const;
     int area(const Paper&, int, part) const;   
     long intensity(const Paper&, int, part) const;     
     vector<int> blob(const Paper& , int) const;
     double olive_moment(const Paper&, int) const;
     void print(ostream&) const;
private: 
     int Xcen, Ycen;     // center of head
     int Xmin, Xmax, Ymin, Ymax;         
};

void stop()
{ cout << "Stop. Enter a letter: "; 
  char ch; cin >> ch; 
}

Paper::Paper() 
{  s1 = ""; s2 = "";
   len1 = 0; len2 = 0;
   depth = 0;
   text = NULL;  // the image
   len = 0;
}

Paper::~Paper()
{ if (text!=NULL) delete text; }

Paper::Paper(const Paper &p)
{  s1 = p.s1; s2 = p.s2;     
   len1 = p.len1; len2 = p.len2;
   depth = p.depth;
   len = p.len; 
   text = new unsigned char[len];
   if (text == NULL)
   { cout << "memory" << endl; stop(); exit(1); }
   for (long i = 0; i < len; i++) text[i] = p.text[i];  
   for (int k = 0; k < p.items.size(); k++) items.push_back(p.items[k]);
//   items = p.items;
} 

Paper::Paper(int l1, int l2)
{ s1 = ""; s2 = "";
  len1 = l1; len2 = l2; len = l1*l2;
  depth = 255;
  text = new unsigned char[len];
  for (long i = 0; i < len; i++) text[i] = 255;           
}                   

Paper &Paper::operator=(const Paper &p)
{ s1 = p.s1; s2 = p.s2;
  len1 = p.len1; len2 = p.len2;
  len = p.len;
  depth = p.depth;

  if (text != NULL) delete text;
  text = new unsigned char[len];
  if (text == NULL)
  { cout << "memory" << endl; stop(); exit(1); }

  for (int i = 0; i < len; i++) text[i] = p.text[i];
//  for (int k = 0; k < p.items.size(); k++) items.push_back(p.items[k]);  
  items = p.items;
  return *this;
}         

void Paper::set_items(const Paper &p)
{ for (int i = 0; i < items.size(); i++)
    items[i] = p.items[i];
}

void Paper::draw_items(int color, int th)
{ cout << "draw_items " << items.size() << endl;  
  for (int i = 0; i < items.size(); i++)
    items[i].draw(*this, color, th);
}

/****************** file processing **************/

void Paper::read_header(istream & fi)
{ string s;
  getline(fi, s1); getline(fi, s2);   
  fi >> len1 >> len2;
  len = len1*len2;
  cout << "Image " << len1 << "x" << len2 
       << "=" << len << endl;
  fi >> depth;
  cout << "Color depth " << depth << endl;
  if (text != NULL) delete text;
  text = new unsigned char[len];
  if (text == NULL)
  { cout << "No memory!" << endl; stop(); exit(1); }
}
                       
void Paper::read_bin(istream& fi)
{ read_header(fi);
  fi.read(reinterpret_cast<char*>(text), len);
}

void Paper::write_header(ostream & fo) const
{ fo << "# Created by p1.exe" << endl;
  fo << len1 << " " << len2 << endl;
  fo << depth;
}        

void Paper::write_bin(ostream & fo) const
{ fo << "P5" << endl;
  write_header(fo);
  fo << endl; 
  fo.write(reinterpret_cast<char*>(text), len);      
}     
/******************** Rect *******************************/

Rect::Rect() 
{ Xcen = 0;  Ycen = 0; 
  Xmin = 0;  Ymin = 0;
  Xmax = 0;  Ymax = 0;
} 
  
Rect::Rect(int x, int y) 
{ Xcen = x; Ycen = y; 
  Xmin = 0;  Ymin = 0;
  Xmax = 0;  Ymax = 0;
}

void Rect::print(ostream &out) const
{ out << getXmin() << "-" << getXmax() << " : " 
      << getYmin() << "-" << getYmax() << endl;
}

void Rect::draw(Paper &p, int color, int th) const
{ // cout << "draw b" << endl;  print();
  int i, j, jth;
  for (j = Ymin; j < Ymax; j++)
     for (jth = 0; jth < th; jth++)
  { p.pixel(Xmin, j + jth) = color;
    p.pixel(Xmax - 1, j + jth) = color;
  }  
  for (i = Xmin; i < Xmax; i++)
    for (jth = 0; jth < th; jth++)
  { p.pixel(i + jth, Ymin) = color;
    p.pixel(i + jth , Ymax - 1) = color;  
  }    
  p.pixel(Xcen, Ycen) = 0; // a dot at the center
  // cout << "draw e" << endl;
}

void Rect::draw_item(const Paper &p, Paper &q) const
{ for (int i = Xmin; i < Xmax; i++)
   for (int j = Ymin; j < Ymax; j++)
     q.pixel(i, j) = p.pixel(i, j);
}

bool Rect::is_inside(int x, int y) const
{ // cout << x << "," << y << " "; print(); stop();
  if (x > Xmin && x < Xmax && y > Ymin && y < Ymax) return true;
  else return false;     
}     

bool Rect::section(const Rect& r) const
{ // cout << "section " << endl;  print(); r.print();
  for (int i = Xmin + 1; i < Xmax; i++)
     for (int j = Ymin + 1; j < Ymax; j++)
  if (r.is_inside(i,j)) return true;   
  return false;
}     

int Rect::area(const Paper& p, int tresh, part cht) const
{  int count = 0;
   int XYmax = Xmax, XYmin = Xmin ;
   if (cht == head) XYmax = Xcen;
   else if (cht == tail) XYmin = Xcen + (Xcen - Xmin);  //???????????????????
   for (int i = XYmin; i < XYmax; i++)
     for (int j = Ymin; j < Ymax; j++)
        if (p.pixel(i, j) > tresh) count++;
   return (cht == head) ? 2*count : count; 
}

long Rect::intensity(const Paper& p, int tresh, part cht) const
{  long sum = 0;
   int dX = Xmax - Xmin;
   int XYmax = Xmax, XYmin = Xmin ;
   if (cht == head) XYmax = Xcen;
   else if (cht == tail) XYmin = Xcen;  //?????????????????????
   for (int i = XYmin; i < XYmax; i++)
     for (int j = Ymin; j < Ymax; j++)
   { int gray = p.pixel(i, j);  
         if (gray > tresh) sum += gray;
   }     
   return (cht == head) ? 2*sum : sum; 
}

vector<int> Rect::blob(const Paper& p, int tresh) const
{ vector<int> vec;
  int maxv = 0;
  for (int i = Xmin + 1; i < Xmax; i++)
  { vec.push_back(0);
    for (int j = Ymin + 1; j < Ymax; j++)      
    { int pp = p.pixel(i, j);
      if (pp > tresh) vec[i - Xmin - 1] += pp;  // ????????????????????
    }  
    if (maxv < vec[i - Xmin - 1]) maxv = vec[i - Xmin - 1];
  }       
  for (int j = 0 ; j < vec.size(); j++) vec[j] = 100*vec[j]/maxv;   
  return vec;
}     

double Rect::olive_moment(const Paper& p, int tresh) const
{ double ol = 0, sum = 0;
  for (int i = Xmin; i < Xmax; i++)
  { int sumi = 0;
    for (int j = Ymin; j < Ymax; j++)      
    { int pp = p.pixel(i, j);
      if (pp > tresh) { sumi += pp;  sum += pp; } //???????????????????     
    }  
    ol = ol + sumi*(i - Xcen);
  }       
  ol = 1.0*ol/sum;
  return ol;     
}       

/****************** Kiril *************************/

void Paper::histo(string name)
{ for (int i=0; i<256; i++) his.push_back(0);
  unsigned int maxh = 0, maxi;
  long i, j;
  for (i=0; i<len; i++) his[text[i]]++;
  for (i=1; i<256; i++) 
     if (his[i]>maxh) { maxh = his[i]; maxi = i; }
  cout << endl << "maxi:maxh = " << maxi << ":" << maxh << endl;  
     
  string name1 = "img/" + name + ".txt";
  ofstream ff(name1.c_str());
  for (i = 0; i < 256; i++) 
  { ff << i << "-" << his[i] << endl; }
  ff.close();
 
  MMAXval = 255;
  i = 255;
  while (i >= 0 && his[i] <= 10) i--;
  MAXval = i - (i - maxi)/3; 
  i = 0;
  while (i < 256 && his[i] <= 10) i++;
  MMINval = (maxi + i)/2; MINval = maxi + 10; // 10; // ????????????
  comet_val = MINval;                         // ???????????? 
  cout << MMINval << " " << MINval << " : " 
       << MAXval << " " << MMAXval << endl; 
  
  const int AAA = 256;
  const int MARGIN = 10;
  Paper h(AAA + 2*MARGIN, AAA + 2*MARGIN);  
  for (i = 0; i < AAA; i++) 
  { double y = static_cast<double>(his[i])*AAA/maxh + MARGIN + 1;
    int j;
    for (j = MARGIN; j < y; j++) 
       h.pixel(i + MARGIN, j) = 0;     
  }  
  string name2 = "img/" + name + "_h.pgm"; 
  ofstream fo(name2.c_str(), ios::binary); // 
  h.write_bin(fo);
  fo.close();
}

void Paper::rectangles() // Kiril
{ int i, j;
  for (i=0; i<len; i++) 
     if (text[i] < MMINval) text[i] = 0;  
  
  for (int i = 1; i < len2; i++)
     for (int j = 1; j < len1; j++)
  { bool yes = false;
    int p = 0;
    while (p < items.size() && !yes)
    { if (items[p].is_inside(j,i)) yes = true; 
      p++;
    }  
//   cout << "yes = " << yes << " i,j=" << i << "," << j << endl;
    if (text[i*len1 + j] > MAXval && !yes) // ?????????????????????? 
    { // cout << "RECT begin" << endl;
      int k1 = i, k2 = i;
      while (k1 < len2-1 && text[k1*len1 + j] > MAXval) k1++;
      while (k2 > 0 && text[k2*len1 + j] > MAXval) k2--;
      int i1 = (k1 + k2)/2;
      k1 = k2 = j;
//      cout << "RECT 1" << endl;

      while (k1 < len2-1 && text[i*len1 + k1] > MAXval) k1++;
      while (k2 > 0 && text[i*len1 + k2] > MAXval) k2--;
      int j1 = (k1 + k2)/2;

      Rect r(j1, i1);      

      int k = j1; 
      while (k < len1-1 && text[i1*len1 + k] > MINval) k++; 
      r.setXmax(k);  
      k = j1; 
      while (k > 0 && text[i1*len1 + k] > MINval) k--; 
      r.setXmin(k);


      cout << r.getXmin() << " && " << r.getXmax() 
                          << " & "<< int(text[i1*len1 + j1])<< endl; 
      k1 = k2 = i1;
      for (int j2 = j1; j2 < r.getXmax(); j2++)
      {  k = i1; 
         while (k < len2-1 && text[k*len1 + j2] > MINval) 
         { // cout << int(text[k*len1 + j2]) << " "; 
         k++; } 
         //cout << "/";
         if (k > k1) k1 = k;     
         k = i1; 
         while (k > 0 && text[k*len1 + j2] > MINval) 
         { // cout << int(text[k*len1 + j2]) << " "; 
         k--; }
         // cout << "|";  
         if (k < k2) k2 = k;
         // stop();
      }
      r.setYmax(k1);
      r.setYmin(k2);  
                 
      j = r.getXmax() + 10; // for next comet; horizontal space b/w comets      
//      r.print();
      items.push_back(r);
//      cout << "RECT end  j = " << j << " i = " << i << endl;  stop();
    }
  }     
}
 
void Paper::reduce()
{ // cout << "begin reduce: " << items.size() << endl;
  vector<int> flag;
  for (int i = 0; i < items.size(); i++) flag.push_back(1); 
  for (int i = 0; i < items.size(); i++)
  {  for (int j = i + 1; j < items.size(); j++)
     if (items[i].section(items[j])) 
    { flag[i] = 0; flag[j] = 0; 
     // cout << i << " | " << j << endl;
    } 
  }  
//for (int i = 0; i < items.size(); i++) cout << flag[i] << " "; cout << endl;
   
  vector<Rect> items2;
  for (int i = 0; i < items.size(); i++) 
    if (flag[i] != 0) items2.push_back(items[i]); 
  // cout << items2.size() << " !!!!!!!!! " << endl; 
  int ii = items.size();   
  for (int i = 0; i < ii; i++) items.pop_back(); 
  // cout << items.size() << "????????????" << endl;
  for (int i = 0; i < items2.size(); i++) items.push_back(items2[i]);
  // cout << items.size() << "  end reduce: " << items.size() << endl; 
}      

void Paper::calculate(string name) const
{ string name1 = "img/" + name + "_c.txt";
  ofstream ff(name1.c_str());
  ff << name << endl;
  for (int i = 0; i < items.size(); i++)
  { ff << "No. " << i << endl;      
    Rect r = items[i];
    r.print(ff); 
    ff << "Comet Length          " << r.getXmax() - r.getXmin() << endl;
    ff << "Comet Height          " << r.getYmax() - r.getYmin() << endl;   
    ff << "Comet Area            " << r.area(*this, comet_val, comet) << endl;
    int TCI = r.intensity(*this, comet_val, comet);
    ff << "Total Comet Intensity " << TCI << endl;
    ff << "Mean Comet Intensity  " << TCI*1.0/r.area(*this, comet_val, comet) 
                                   << endl;
    int HD = 2*(r.getXcen() - r.getXmin());                               
    ff << "Head Diameter         " << 2*(r.getXcen() - r.getXmin()) << endl;
    ff << "Head Area             " << r.area(*this, comet_val, head) << endl;
    int THI = r.intensity(*this, comet_val, head);
    ff << "Total Head Intensity  " << THI << endl;
    ff << "Mean Head Intensity   " << THI*1.0/r.area(*this, comet_val, head) 
                                   << endl;
    ff << "%DNA in Head          " << 100.0*THI/TCI << endl; 
    int TL = r.getXmax() - r.getXmin() - HD;
    ff << "Tail Length           " << TL << endl;
    ff << "Tail Area             " << r.area(*this, comet_val, tail) << endl;
    int TTI = r.intensity(*this, comet_val, tail) - THI/2;
    ff << "Total Tail Intensity  " << TTI << endl;
    ff << "Mean Tail Intensity   " << TTI*1.0/r.area(*this, comet_val, tail) 
                                   << endl;
    ff << "%DNA in Tail          " << 100.0*TTI/TCI << endl; 
    ff << "Tail Moment           " << 1.0*TTI/TCI*TL << endl;
    ff << "Olive Moment          " << r.olive_moment(*this, comet_val) << endl;
  }
  ff.close();    
}   

void Paper::draw_blobs(string name)
{ string name1 = "img/" + name + "_b.txt";
  ofstream ff(name1.c_str());
  for (int i = 0; i < items.size(); i++)
  { ff << "No. " << i << " ";
    vector<int> a = items[i].blob(*this, comet_val);
    for (int j = 0 ; j < a.size(); j++) ff << a[j] << " ";   
    ff << endl;
  
  /*  ONLY ONE PICTURE !!!!!!!!!!????????!!!!!!!!    */
    int AAA = a.size();
    int x0 = items[i].getXcen() - items[i].getXmin();
    const int MARGIN = 10;
    Paper h(AAA + 2*MARGIN, 100 + 2*MARGIN);  
    for (int ii = 0; ii < x0; ii++) 
       h.pixel(ii + MARGIN, a[ii] + MARGIN + 1) = 0;  
    for (int ii = x0; ii < 2*x0; ii++) 
    {  h.pixel(ii + MARGIN, a[ii] - a[2*x0 - ii] + MARGIN + 1) = 0; 
       h.pixel(ii + MARGIN, a[2*x0 - ii] + MARGIN + 1) = 0;
       h.pixel(ii + MARGIN, a[ii] + MARGIN + 1) = 0;      
    }     
    for (int ii = 2*x0; ii < AAA; ii++) 
       h.pixel(ii + MARGIN, a[ii] + MARGIN + 1) = 0;         
              
    string name2 = "img/" + name + "_b.pgm"; 
    ofstream fo(name2.c_str(), ios::binary); // 
    h.write_bin(fo);
    fo.close();
  }  
  ff.close();      
}       

void comets(string name)
{ Paper p;
  string name1 = "img/" + name + ".pgm"; 
  ifstream fi(name1.c_str(), ios::binary);
  p.read_bin(fi);
  fi.close();
// stop();
  
  cout << "Processing ... " << endl;
  p.histo(name);
  
  p.rectangles(); 
//  cout << "end rectangles" << endl;
  
  Paper q(p);
  q.draw_items(255, 2);
 
  string name2 = "img/" + name + "_1.pgm";
  ofstream fo(name2.c_str(), ios::binary);
  q.write_bin(fo);
  fo.close();
 
  p.reduce();
  p.draw_blobs(name);
   
  q = p;
  q.draw_items(250, 2);
  name2 = "img/" + name + "_2.pgm";
  fo.open(name2.c_str(), ios::binary);
  q.write_bin(fo);
  fo.close();
  
  p.calculate(name);
/*
  string name2 = "img/" + name + "_1.pgm";
  ofstream fo(name2.c_str(), ios::binary);
  p.write_bin(fo);
  fo.close();
*/
  stop();
}

int main(int argc, char *argv[])
{  if (argc < 2) { cout << "Usage: com03 file" << endl; exit(1); }
   string filename = static_cast<string>(argv[1]);
   comets(filename);
   return 0;
}
