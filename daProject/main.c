//Placement code 

#include<sys/time.h>
#include<sys/resource.h>
#include<unistd.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstdlib>
#include <math.h>


using namespace std;

//Declare the Data Structures for cell and net
struct net;
struct cell;

// Data structure to create linked list of nets (Netlists)
struct net_node {
	net *NP;
	net_node *next;
};

// Data structure to create linked list of cells (Cellists)
struct cell_node {
	cell *CP;
	cell_node *next;
};

// Data structure for cells and pads
struct cell{
	string Cell_name;
	long int Cell_area;
	long int x,y;    //x and y co-ordinates of lower left boundary
	net_node *head, *tail;  //Pointers to store the value of head and tail pointers of Netlist
};

// Data structure for nets
struct net{
	long int Net_number;
	float wire;
	bool crit;
	cell_node *head, *tail;  //Pointers to store the value of head and tail pointers of Netlist
} ;

map<string, int> Cell_id;    //Map each cell name to its index number in the array
vector <cell> Cell ;  //Array of data structure cell -> Cell[i]
vector <net> Net;    //Array of data structure net -> Net[j]
long int i,j ; 
string word;
ifstream fp_are, fp_net;
fstream  fp_place,fp_out,fp_mat;
long int number_of_modules=0 ,number_of_nets=0 ,number_of_cells=0;
float area , width1, height1,width,height;
float wirelength,wirelength_ini,wirelength_final,wirelength_new,wirelength_old,delta;
float Tstart,Temp;

int main (int argc,char *argv[]){
rusage time;
void display();
float wirelength_change(long int,long int);

	//Creating output file
	string file_name;
	if(argc>1) file_name=argv[1]; else file_name="out.txt";
	string  f_without=file_name.substr(0,file_name.rfind("."));
	const std:: string  pre="Pai_Syed_Placement_";
	const std:: string  pre2="Pai_Syed_Placement_summary_";
	const std:: string  pre3="Pai_Syed_matlab_";
	const std:: string post=".txt";

	//File2 for storing placement information
	fp_place.open((pre + f_without+ post).c_str(), fstream::out | fstream::in | fstream::trunc);
	fp_place.close();
	fp_place.open((pre + f_without+ post).c_str(), fstream::out | fstream::in | fstream::trunc);
	fp_out.open((pre2 + f_without+ post).c_str(), fstream::out | fstream::in | fstream::trunc);
	fp_out.close();
	fp_out.open((pre2 + f_without+ post).c_str(), fstream::out | fstream::in | fstream::trunc);
	fp_mat.open((pre3 + f_without+ post).c_str(), fstream::out | fstream::in | fstream::trunc);
	fp_mat.close();
	fp_mat.open((pre3 + f_without+ post).c_str(), fstream::out | fstream::in | fstream::trunc);
	
	//Read .net to get number of cells and nets
	fp_net.open(file_name.c_str()); 
	fp_net>>number_of_nets;
	fp_net>>number_of_nets;
	fp_net>>number_of_nets;
	fp_net>>number_of_modules;
	fp_net>>number_of_cells;
	fp_net.close();
	
	//Resize cells and nets based on their number
	Cell.resize(number_of_modules); 
	Net.resize((number_of_nets)+1);
	
	//Create Cells and pads using .are file
	string cells_name;
	//if(argc>2) cells_name=argv[2]; else cells_name="cells.txt";
	fp_are.open(argv[2]); 
	i=0;
	
	long int area;
	while ((fp_are >> word) && (fp_are>>area)){
		Cell[i].Cell_name= word;  
		Cell[i].Cell_area=area;
		Cell[i].head=NULL; 
		Cell_id[word] = i; //Map name of cell to its index in the array
		i++;
	}
	fp_are.close(); 

	
	// CODE TO STORE ELEMENTS INSIDE CELL_LIST AND NET_LIST ARRAYS
	fp_net.open(argv[1]);
	string prev_word , next_word; // Temp variables to store previous and next word
	cell *Cell_ptr; net* Net_ptr;// Temp pointers to Cell and Net
	cell_node *c_node; net_node *n_node;// Temp pointers to Cell_node and Net_node
	//Ignoring first 4 lines
	fp_net >> prev_word;
	fp_net >> prev_word;
	fp_net >> prev_word;
	fp_net >> prev_word;
	//Store first word after ignored lines
	fp_net >> prev_word;
	fp_net >> word;
	j=0;
	
	while(fp_net >> next_word){
		//If a new Net is found . Add previous cell to Netlist and add net to Cellist of previous cell
		if ((word =="s")){
			j++;
			Net[j].Net_number=j; //Create new Net[j]
			Net[j].head=NULL; // Create empty list	
			i = Cell_id[prev_word]; //Find the index of the Cell represented by prev_word 
			Cell_ptr = &Cell[i];
			
			c_node = new cell_node;  //Create new cell node
			c_node->CP = Cell_ptr;   //Store address of cell in the cell node
			c_node->next = NULL;    
			if (Net[j].head == NULL) {
				Net[j].head = Net[j].tail = c_node; //Head and tail of netlist is newly created cell node
			} else {
				Net[j].tail->next = c_node; 
				Net[j].tail = c_node;      
			}
			
			Net_ptr = &(Net[j]);      //Get addres of current Net
			n_node = new net_node;  //Create a new net node
			n_node->NP = Net_ptr;   //Store the address of net in net node
			n_node->next = NULL;
			if (Cell[i].head == NULL) {
				Cell[i].head = Cell[i].tail = n_node; //Head and tail of Cellist is newly created cell node
			} else {
				Cell[i].tail->next = n_node;  //Add a new element to Celllist
				Cell[i].tail = n_node;        //Point the tail of the Cellist to the new element
			}
		} 
		
		//If old Net is continued . Add the cell to Netlist and add the current net to Cellist of the cell
		if ((word !="s") && (word !="1") &&(word != "l") &&(next_word !="s") ){
			i = Cell_id[word];  //Find the index of the Cell represented by word 
			Cell_ptr = &Cell[i];		
			c_node = new cell_node;  //Create new cell node
			c_node->CP = Cell_ptr;   //Store address of cell in the cell node
			c_node->next = NULL;
			if (Net[j].head == NULL) {
				Net[j].head = Net[j].tail = c_node;
			} else {
				Net[j].tail->next = c_node; //Add a new element to netlist
				Net[j].tail = c_node;        //Point the tail of the netlist to the new element
			}
			
			Net_ptr = &(Net[j]);  //Get address of current net
			n_node = new net_node;  //Create a new net node
			n_node->NP = Net_ptr;   //Store the address of net in net node
			n_node->next = NULL;
			if (Cell[i].head == NULL) {
				Cell[i].head = Cell[i].tail = n_node;//Head and tail of Cellist is newly created cell node
			} else {
				Cell[i].tail->next = n_node;  //Add a new element to Celllist
				Cell[i].tail = n_node;        //Point the tail of the Cellist to the new element
			}
		}
		prev_word=word;
		word=next_word;
	}
	fp_net.close();
		
	//========================GETTING PARAMETERS BEFORE PARTITION =================================
	
	//Place randomly
	area = number_of_cells*4*16*2;
	width=width1=height1= sqrt(area);
	area=area+width1*16;
	width=width1=height1= sqrt(area);
	long int xcor,ycor;
	xcor=-4;
	ycor=16;
	for(i=0;i<(number_of_modules);i++)
	{
	if(xcor>=width1)
	{
	xcor=-4;
	ycor=ycor+32;
	}
	Cell[i].x=xcor+4;
	xcor=xcor+4;
	if(width<(xcor+4))
	width=xcor+4;
	Cell[i].y=ycor;
	}
	height=ycor+32;		


	//Find wirelength 
	wirelength=0;
	long int xmin,xmax,ymin,ymax;
	for(j=1;j<=number_of_nets;j++)
	{
	xmax=ymax=0;
	xmin=width;
	ymin=height;
	c_node=Net[j].head;
	while(c_node!=NULL)
	{
	if(xmin>c_node->CP->x)
	xmin=c_node->CP->x;
	if(xmax < c_node->CP->x)
	xmax= c_node->CP->x ;
	if(ymin>c_node->CP->y)
	ymin=c_node->CP->y;
	if(ymax < c_node->CP->y)
	ymax= c_node->CP->y;
	c_node=c_node->next;
	}
	Net[j].wire=(xmax-xmin+4)+(ymax-ymin+16);
	wirelength=wirelength+ Net[j].wire;
	}
	wirelength_ini = wirelength;
	
	
	//======Computing Tstart============
	long int rand1,rand2;
	long int xtemp,ytemp;
	float wire_max,wire_min;
	wire_max = wire_min = wirelength;

	for(i=0;i<(0.8*number_of_modules);i++)
	{
	for(j=0;j<20;j++)
	{
	rand1=rand()% number_of_modules + 0;
	rand2=rand()% number_of_modules + 0;
	xtemp=Cell[rand1].x;
	Cell[rand1].x=Cell[rand2].x;
	Cell[rand2].x=xtemp;
	ytemp=Cell[rand1].y;
	Cell[rand1].y=Cell[rand2].y;
	Cell[rand2].y=ytemp;
	wirelength=wirelength+ wirelength_change(rand1,rand2);
	if(wirelength>wire_max)
	wire_max=wirelength;
	if(wirelength<wire_min)
	wire_min=wirelength;
	}
	}
	Tstart=(wire_min-wire_max)/log(0.8);


	

	//========================SIMULATED ANNEALINH ROUTINE BEGINS HERE=================================
	long int iter=1;
	long int inner_count =0;
	long int same_cost=0;
	bool end=0;
	float winw,winh,constw,consth;
	winw=2*width;
	winh=2*height;
	constw=winw/log(Tstart);
	consth=winh/log(Tstart);

	
	for(Temp=Tstart;((winw>=4 && winh>=16)&& end==0);(Temp=0.94*Temp))
	{
	for(inner_count=0;inner_count<(0.8*number_of_modules);inner_count++)
	{
	//Step1:Find a solution
	random2:
	rand1=rand()% number_of_modules + 0;
	rand2=rand()% number_of_modules + 0;
	//Check whether second cell is within defined range
	if ((abs(Cell[rand1].x - Cell[rand2].x) > (winw/2) ) || (abs(Cell[rand1].y - Cell[rand2].y) > (winh/2) ) )
	{
	//cout<<"stuck\n";
	goto random2;
	}
	//Swap cells
	xtemp=Cell[rand1].x;
	Cell[rand1].x=Cell[rand2].x;
	Cell[rand2].x=xtemp;
	ytemp=Cell[rand1].y;
	Cell[rand1].y=Cell[rand2].y;
	Cell[rand2].y=ytemp;
	
	//Find change in wirelength
	wirelength_new=wirelength + wirelength_change(rand1,rand2);
	delta= wirelength - wirelength_new;
	float prob;
	prob=(rand()%10);
	prob=1/prob;
	
	//Step 2:Is solution acceptable ? If yes, store it 
	if(exp(delta/Temp)>=1)
	{
	//cout<<"Accept"<<"\n";
	wirelength= wirelength_new;
	}
	
	else if (exp(delta/Temp)>=prob)
	{
	//cout<<"Accept with prob"<<"\n";
	wirelength= wirelength_new;;
	}
	
	else 
	{
	//cout<<"Reject"<<"\n";
	xtemp=Cell[rand1].x;
	Cell[rand1].x=Cell[rand2].x;
	Cell[rand2].x=xtemp;
	ytemp=Cell[rand1].y;
	Cell[rand1].y=Cell[rand2].y;
	Cell[rand2].y=ytemp;
	wirelength_change(rand1,rand2);
	}
	//Check if there is any difference in wirelength
	if(wirelength_old=wirelength)
	same_cost++;
	else
	same_cost=0;
	//cout<<same_cost<<"\t";
	wirelength_old=wirelength;
	//If cost remains same after a series of n executions,end simulated annealing
	if(same_cost>=(number_of_modules*500))
	end=1;
	iter++;
	}
	//Update range delimiter 
	winw=constw*log(Temp);
	winh=winh*log(Temp);
	}
	

	float red;
	red= wirelength_ini-wirelength;
	red=red/wirelength_ini;
	red=red*100;
	fp_place<<"Cell Name\t<x>\t<y>\n";
	display();
	//Calculate time
	getrusage(RUSAGE_SELF,&time);
	fp_out<<"Aspect ratio         :"<<"\t"<<(float(height)/float(width))<<"\n";
	fp_out<<"wire length initial  :"<<"\t"<<wirelength_ini<<"\n";
	fp_out<<"wire length final    :"<<"\t"<<wirelength<<"\n";
	fp_out<<"%age reduction       :\t"<<red<<"\n";
	fp_out<<"Execution time       : "<<(double)(1.0*time.ru_utime.tv_sec+0.000001*time.ru_utime.tv_usec)<<"seconds"<<"\n";
	

  fp_place.close();	
  fp_out.close();
  fp_mat.close();
}//end main


//FUNCTIONS	
void display()
{
	for(i=0;i<(number_of_modules);i++)
	{
	fp_place<<Cell[i].Cell_name<<"\t"<<Cell[i].x<<"\t"<<Cell[i].y<<"\n";
	fp_mat<<Cell[i].x<<"\t"<<Cell[i].y<<"\n";
	}
}

float wirelength_change(long int a,long int b)
{
long int xmax,xmin,ymax,ymin;
float delta=0;
cell_node* c_node;
net_node* n_node;
n_node=Cell[a].head;
while(n_node!=NULL)
{
xmax=ymax=0;
xmin=width;
ymin=height;
c_node=n_node->NP->head;
while(c_node!=NULL)
{
if(xmin>c_node->CP->x)
xmin=c_node->CP->x;
if(xmax < c_node->CP->x)
xmax= c_node->CP->x ;
if(ymin>c_node->CP->y)
ymin=c_node->CP->y;
if(ymax < c_node->CP->y)
ymax= c_node->CP->y;
c_node=c_node->next;
}
delta=delta-n_node->NP->wire+(xmax-xmin+4)+(ymax-ymin+16);
n_node->NP->wire=(xmax-xmin+4)+(ymax-ymin+16);
n_node->NP->crit=1; // Mark the net as wlength has been computed
n_node=n_node->next;
}

n_node=Cell[b].head;
while(n_node!=NULL)
{
xmax=ymax=0;
xmin=width;
ymin=height;
c_node=n_node->NP->head;
while(c_node!=NULL)
{
if(xmin>c_node->CP->x)
xmin=c_node->CP->x;
if(xmax < c_node->CP->x)
xmax= c_node->CP->x ;
if(ymin>c_node->CP->y)
ymin=c_node->CP->y;
if(ymax < c_node->CP->y)
ymax= c_node->CP->y;
c_node=c_node->next;
}
if(n_node->NP->crit!=1) //compute change in wirelength only if net is not a part of cell a
{
delta=delta-n_node->NP->wire+(xmax-xmin+4)+(ymax-ymin+16);
n_node->NP->wire=(xmax-xmin+4)+(ymax-ymin+16);
}
n_node=n_node->next;
}
n_node=Cell[a].head;
while(n_node!=NULL)
{
n_node->NP->crit=0;
n_node=n_node->next;
}
return(delta);
}



