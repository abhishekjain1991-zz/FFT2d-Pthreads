// Threaded two-dimensional Discrete FFT transform
// Abhishek Jain
// ECE8893 Project 2


#include <iostream>
#include <string>
#include <math.h>
#include "pthread.h"
#include "Complex.h"
#include "InputImage.h"
const int N=1024;
// You will likely need global variables indicating how
// many threads there are, and a Complex* that points to the
// 2d image being transformed.

using namespace std;

// Function to reverse bits in an unsigned integer
// This assumes there is a global variable N that is the
// number of points in the 1D transform.
unsigned ReverseBits(unsigned v)
{ //  Provided to students
  unsigned n = N; // Size of array (which is even 2 power k value)
// unsigned n = N; // Size of array (which is even 2 power k value)
  unsigned r = 0; // Return value
   
  for (--n; n > 0; n >>= 1)
    {
      r <<= 1;        // Shift return value
      r |= (v & 0x1); // Merge in next bit
      v >>= 1;        // Shift reversal value
    }
  return r;
}

void Transform1d(int f);
int *a=new int [8];
int b[N];
int  nThreads = 16;
class  GoodBarrier {
	public:
	GoodBarrier(int P0);	 
	void Enter(int  myId);	
	private:
	int 	P;
	int 	count;	 
	int  FetchAndDecrementCount();
	pthread_mutex_t countMutex;
	bool* localSense;	
	bool 	globalSense;	
	};
GoodBarrier::GoodBarrier(int  P0)
: P(P0), count(P0)
	{

	pthread_mutex_init(&countMutex, 0);
	
	localSense =  new bool[P];
for (int i =  0; i <  P; ++i) localSense[i]  =  true;
//Initialize global  sense
	globalSense =  true;
	}

	void GoodBarrier::Enter(int  myId)
	{ 
	localSense[myId] =  !localSense[myId];	// Toggle private  sense   variable
	if (FetchAndDecrementCount() ==  1)
	{ // All  threads here, reset count and  toggle  global  sense
	count =  P;
	globalSense =  localSense[myId];
	}
	else
	{
	while (globalSense != localSense[myId]) {	} // Spin
	}

	}


	int  GoodBarrier::FetchAndDecrementCount()
	{ 
pthread_mutex_lock(&countMutex);
int myCount  =  count;
	count--;
	pthread_mutex_unlock(&countMutex);
	return  myCount;
	}
//int  startCount = nThreads;
GoodBarrier* activeMutex;
 pthread_mutex_t exitMutex;
 pthread_cond_t exitCondition;
Complex *img;
int w;
int h;
Complex* copy1;
Complex* W;
Complex* trans;
int count_z=0;
int sign=1;
void* transform(void* v)
{unsigned long myid = (unsigned long ) v;
//unsigned long b = 64*myid;
int b=(int)(64*myid);
pthread_mutex_lock(&exitMutex);
for(int i=b;i<b+64;i++)
{

Transform1d(N*i);
}
cout<<"thread "<<myid<<"work done"<<endl;

	
pthread_mutex_unlock(&exitMutex);

activeMutex->Enter(myid);

return 0;
}

int main(int argc, char** argv)
{

  string fn2("Tower.txt"); // default file name
  if (argc > 1) fn2 = string(argv[1]);  // if name specified on cmd line
const char* fn = fn2.c_str();
InputImage image=InputImage(fn);
  // MPI initialization here
  //Transform2D(fn.c_str()); // Perform the transform.
 //Complex*
  img = image.GetImageData();
//int
 w = image.GetWidth();
  //int
   h = image.GetHeight();
 int size=w*h;
//Complex*
 W= new Complex[w];
//Complex*
 copy1 = new Complex[size];
trans=new Complex[size];


for(int i=0;i<size;i++)
copy1[i]=Complex(cos(0),sin(0))-Complex(cos(0),sin(0));

for(int  i=0;i<N;i++)
{
b[i]=i;

}

for(int i=0;i<N;i++)
{
b[i]=ReverseBits(b[i]);

}

for(int i=0;i<N/2;i++)
{W[i]=Complex(cos(((2*M_PI)*i)/N),-sin(((2*M_PI)*i)/N));

}
 activeMutex  = new GoodBarrier(nThreads+1);

pthread_mutex_init(&exitMutex, 0);

for (int i = 0; i < nThreads; ++i)
    {
      pthread_t t;
      pthread_create(&t, 0, transform,  (void*)i);
    }



activeMutex->Enter(nThreads);

cout<<"All threads done and 1 dimensional fourier Transform is ready"<<endl;
//image.SaveImageData("Output1dmine.txt",copy1,w,h);
//----------------Transposing-------------------------------------------------------//
count_z++;
for(int i=0,k=0;i<h;i++)
{
for(int j=i;j<size;j=j+w,k++)
{
trans[k]=copy1[j];
}
}
for(int i=0;i<size;i++)
{
copy1[i]=trans[i];
}

//------------------Done transposing doing the 2d transform--------------------------------------------------------//

pthread_mutex_init(&exitMutex, 0);

activeMutex  = new GoodBarrier(nThreads+1);
for (int j= 0; j < nThreads; ++j)
 {
 pthread_t t;
 pthread_create(&t, 0, transform,  (void*)j);
 }


 activeMutex->Enter(nThreads);

for(int i=0,k=0;i<h;i++)
{
for(int j=i;j<size;j=j+w,k++)
{
trans[k]=copy1[j];
}
}
for(int i=0;i<size;i++)
{
copy1[i]=trans[i];
}

cout<<"All threads done and 2 dimensional fourier Transform is ready"<<endl;
//image.SaveImageData("Output2dmine2.txt",copy1,w,h);


//--------------------inversing ---------------------------------//
for(int i=0;i<N/2;i++)
{W[i]=Complex(cos(((2*M_PI)*i)/N),sin(((2*M_PI)*i)/N));
//W[i].Print();
cout<<endl;
}

Complex inverse = Complex(N,0);
activeMutex  = new GoodBarrier(nThreads+1);

pthread_mutex_init(&exitMutex, 0);
for (int i = 0; i < nThreads; ++i)
 {
 pthread_t t;
 pthread_create(&t, 0, transform,  (void*)i);
 }


 activeMutex->Enter(nThreads);

for(int i=0;i<size;i++)
copy1[i]=copy1[i]/inverse;
cout<<"All threads done and inverse  1 dimensional fourier Transform is ready"<<endl;
for(int i=0,k=0;i<h;i++)
{
for(int j=i;j<size;j=j+w,k++)
{
trans[k]=copy1[j];
}
}
for(int i=0;i<size;i++)
{
copy1[i]=trans[i];
}

activeMutex  = new GoodBarrier(nThreads+1);

pthread_mutex_init(&exitMutex, 0);
for (int i = 0; i < nThreads; ++i)
 {
 pthread_t t;
 pthread_create(&t, 0, transform,  (void*)i);
 }


 activeMutex->Enter(nThreads);

for(int i=0;i<size;i++)
copy1[i]=copy1[i]/inverse;
cout<<"All threads done and inverse  2 dimensional fourier Transform is ready"<<endl;
for(int i=0,k=0;i<h;i++)
{
for(int j=i;j<size;j=j+w,k++)
{
trans[k]=copy1[j];
}
}
for(int i=0;i<size;i++)

{
copy1[i]=trans[i];
}
image.SaveImageData("InverseOutput_again.txt", copy1,w,h);


}  
  
void Transform1d(int f)
{if(count_z==0)
{
for(int i=f,j=0;i<(N+f);i++,j++)
{
copy1[i]=img[f+b[j]];

}
}
else
{for(int i=f,j=0;i<(N+f);i++,j++)
{
trans[i]=copy1[f+b[j]];

}
for(int i=f;i<(N+f);i++)
{
copy1[i]=trans[i];
}
}

int  d;
Complex temp;
for(int  npoints=2;npoints<=N;npoints=(npoints*2))
{
        for(int  x=0,i=f;x<N/npoints;x++,i=i+npoints)
        {
                for(int  y=0,z=i,g=0;y<npoints/2;y++,z++)
                {d=z+(npoints/2);
                 temp=copy1[z];
                 copy1[z]=copy1[z]+(W[g]*(copy1[d]));
                 copy1[d]=temp-(W[g]*(copy1[d]));
                 g=g+(N/npoints);
                  }



        }
}

}
