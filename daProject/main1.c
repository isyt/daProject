//partitioning code

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
	char part;
	bool move;              //if move=1 , cell has been moved in the iteration
	long int Cell_width;
	long int Cell_height;
	net_node *head, *tail;  //Pointers to store the value of head and tail pointers of Netlist
};

// Data structure for nets
struct net{
	long int Net_number;
	bool critical;            //critical=1 implies that net is a part of cutset or external net
	cell_node *head, *tail;  //Pointers to store the value of head and tail pointers of Netlist
} ;

map<string, int> Cell_id;    //Map each cell name to its index number in the array
vector <cell> Cell ;  //Array of data structure cell -> Cell[i]
vector <net> Net;    //Array of data structure net -> Net[j]
long int i,j ; 
string word;
ifstream fp_are, fp_net;
fstream fp_out , fp_plot ,fp_part;
long int number_of_modules=0 ,number_of_nets=0 ,number_of_cells=0;
long int total_area=0;
long int delta_area; //Delta area on each side of partition
long int psi_ini,psi_new,psi_mean,psi_max,psi_min, psi_final,psi_delta;
float Tstart,Temp;

int main (int argc,char *argv[]){
rusage time;
long int cutset();
long int cutset_change(long int);
void check_crit(long int);
void unmove();
void  display();

	//Creating output file
	string file_name;
	if(argc>1) file_name=argv[1]; else file_name="out.txt";
	string  f_without=file_name.substr(0,file_name.rfind("."));
	const std:: string  pre="team_25_";
	const std:: string post=".txt";
	const std:: string  pre2="Pai_Syed_Plot_";
	const std:: string  pre3="Pai_Syed_Part_summary_";

	//File2 for storing iteration information
	fp_plot.open((pre2 + f_without+ post).c_str(), fstream::out | fstream::in | fstream::trunc);
	fp_plot.close();
	fp_plot.open((pre2 + f_without+ post).c_str(), fstream::out | fstream::in | fstream::trunc);
	//File3 for storing partition summary
	fp_part.open((pre3 + f_without+ post).c_str(), fstream::out | fstream::in | fstream::trunc);
	fp_part.close();
	fp_part.open((pre3 + f_without+ post).c_str(), fstream::out | fstream::in | fstream::trunc);
	 
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
		total_area = total_area + area;
		Cell[i].move=0;
		Cell[i].head=NULL; 
		Cell_id[word] = i; //Map name of cell to its index in the array
		i++;
	}
	fp_are.close(); 
	//TWEAK1
	delta_area= 0.05* total_area;
	
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
	
	//Partition randomly
	long int area_A ,area_B , cells_A, cells_B;
	area_A = area_B = cells_A = cells_B = 0;
	for (i=0;i<number_of_modules ;i=i+2)
	{
	Cell[i].part='A';
	area_A= area_A + Cell[i].Cell_area;
	cells_A ++;
	if(area_A > ((total_area/2) + delta_area))
	{
	Cell[i].part='B';
	area_B= area_B + Cell[i].Cell_area;
	area_A= area_A - Cell[i].Cell_area;
	cells_A-- ; cells_B++;
	}
	Cell[i+1].part='B';
	area_B= area_B + Cell[i+1].Cell_area;
	cells_B++;
	if(area_B >((total_area/2) + delta_area))
	{
	Cell[i+1].part='A';
	area_A= area_A + Cell[i+1].Cell_area;
	area_B= area_B - Cell[i+1].Cell_area;
	cells_B--; cells_A++;
	}
	}


	//Get number of cells on each side 
	 cells_A = cells_A - ((number_of_modules - number_of_cells)/2);
	 cells_B = cells_B - (( number_of_modules - number_of_cells)/2);
	 
	//Ensure that partitioned pads will never move in the iterations
	for(i=(number_of_cells+1);i<(number_of_modules);i++)
	Cell[i].move=1;

	//Find critical nets + cutset 
	char tp;
	psi_ini =0;
	for(j=1;j<=number_of_nets;j++)
	{
	c_node=Net[j].head;
	tp= c_node->CP->part;
	while(c_node!=NULL)
	{
	if(c_node->CP->part != tp)
	Net[j].critical=1; 
	c_node=c_node->next;
	}
	if (Net[j].critical==1)
	psi_ini++;
	}
	psi_max= psi_ini; psi_min= psi_ini; psi_final=psi_ini;
	

	vector <int>psi_table; //Array to store cutset value for each swap
	psi_table.resize(number_of_cells/2);
	j=0;
	long int rand1,rand2,rand3,rand4;

	//TWEAK2
	for(i=0;i<min((cells_A*0.8),(cells_B*0.8));i++)
	{
	//Find a random unmoved cell from A	
	rand1_cell:
	rand1=rand()% number_of_cells + 0;   //Find random cell within allowable cell index
	if((Cell[rand1].part!='A') || (Cell[rand1].move==1))
	goto rand1_cell;
	

	//Find a random unmoved cell from B
	rand2_cell:
	rand2=rand()% number_of_cells + 0;
	if((Cell[rand2].part!='B') || (Cell[rand1].move==1))
	goto rand2_cell;
	
	//Is area okay for move ?	
	if (((Cell[rand1].Cell_area+ area_B-Cell[rand2].Cell_area) > ((total_area/2)+delta_area)) || ((Cell[rand2].Cell_area+ area_A-Cell[rand1].Cell_area) > ((total_area/2)+delta_area)))
	goto rand1_cell;
		
	//Move cells and update area of partitions and find cutset  
	area_B=area_B- Cell[rand2].Cell_area+  Cell[rand1].Cell_area;
	area_A=area_A- Cell[rand1].Cell_area + Cell[rand2].Cell_area;
	Cell[rand1].move=1; Cell[rand1].part='B';
	psi_delta=cutset_change(rand1);
	Cell[rand2].move=1; Cell[rand2].part='A';
	psi_delta=psi_delta+cutset_change(rand2);
	psi_new=psi_final+psi_delta;
	psi_final=psi_new;
	
	if(psi_new>psi_max)
	psi_max=psi_new;
	if(psi_new<psi_min)
	psi_min=psi_new;
	psi_table[j]=psi_new;j++;
	}

	int array=j; //Number of swaps done = size of array;
	//TWEAK3 - value inside log	
	//Find start temperature
	psi_mean=0;
	float Tstd=0;
	for(j=0;j<array;j++)
	{
	psi_mean = psi_mean+(psi_min-psi_table[j]);
	}
	psi_mean=psi_mean/(array);
	Tstd=psi_mean/log(0.8);
	Tstart=(psi_min-psi_max)/log(0.8);
	//fp_part<<"Tstart=    "<<Tstart<<"\n";
	
	//Mark all cells as unmoved 
	unmove();
	
	//fp_part<<"psi initial\t"<<psi_final<<"\n";
	psi_ini =psi_final;

	
	//========================PARTITION ROUTINE BEGINS HERE=================================
	long int iter=1;
	long int inner_count =0;
	long int random;
	for(Temp=Tstart;Temp>0.001;(Temp=0.96*Temp))
	{
	//fp_plot<<"*******Temp\t"<<Temp<<"*************\n";
	//TWEAK4 - search space
	for(inner_count=0;inner_count<min((cells_A*0.8),(cells_B*0.8));inner_count++)
	{
	//Step1:Find a solution
	//Find a random unmoved cell from A
	random=0;	
	rand3_cell:
	if(random>(min(cells_A,cells_B))) 
	goto next;
	rand3=rand()% number_of_cells + 0;   //Find random cell within allowable cell index
	if((Cell[rand3].part!='A') || (Cell[rand3].move==1))
	goto rand3_cell;
	
	//Find a random unmoved cell from B
	rand4_cell:
	rand4=rand()% number_of_cells + 0;
	if((Cell[rand4].part!='B') || (Cell[rand4].move==1))
	goto rand4_cell;
	
	//Is area okay for move ?	
	if (((Cell[rand3].Cell_area+ area_B-Cell[rand4].Cell_area) > ((total_area/2)+delta_area)) || ((Cell[rand4].Cell_area+area_A-Cell[rand3].Cell_area) > ((total_area/2)+delta_area)))
	{ random++;goto rand3_cell;}
		
	//Move cells and update area
	area_A=area_A - Cell[rand3].Cell_area + Cell[rand4].Cell_area;
	area_B=area_B - Cell[rand4].Cell_area + Cell[rand3].Cell_area;	
	Cell[rand3].move=1; Cell[rand3].part='B';
	psi_delta=cutset_change(rand3);
	Cell[rand4].move=1; Cell[rand4].part='A';
	psi_delta=psi_delta + cutset_change(rand4);	
	psi_new=psi_final + psi_delta;
	//fp_plot<<iter<<"\t"<<psi_new<<"\t";
	fp_plot<<Temp<<"\t"<<psi_new<<"\t";
	
	float prob;
	prob=(rand()%10);
	prob=1/prob;

	//Step 2:Is solution acceptable ? If yes, store it 
	if(exp((psi_final-psi_new)/Temp)>=1)
	{
	//fp_plot<<"This is a reduction\n";
	fp_plot<<psi_new<<"\n";
	psi_final=psi_new;
	}
	
	//TWEAK5 - what probability to accept ?
	//else if (exp((psi_final-psi_new)/Temp)>=0.6)
	else if (exp((psi_final-psi_new)/Temp)>=prob)
	{
	//fp_plot<<"Accept with prob\t"<<exp((psi_final-psi_new)/Temp)<<"\n";
	fp_plot<<psi_new<<"\n";
	psi_final=psi_new;
	}
	
	else 
	{
	//fp_plot<<"Reject with Prob\t"<<exp((psi_final-psi_new)/Temp)<<"\n";
	fp_plot<<psi_final<<"\n";
	area_A=area_A + Cell[rand3].Cell_area - Cell[rand4].Cell_area;
	area_B=area_B + Cell[rand4].Cell_area - Cell[rand3].Cell_area;
	Cell[rand3].part='A';
	check_crit(rand3);
	Cell[rand4].part='B';
	check_crit(rand4);
	}
	iter++;
	}
	//Mark all cells as unmoved
	next:
	unmove();
	}
	
	//Calculate time
	getrusage(RUSAGE_SELF,&time);

	fp_part<<"*********  Partition summary  *******\n";
	//display();
	float red;
	red = float(psi_ini-psi_final)/psi_ini;
	red=red*100;
	float rat_cut,size_A,size_B;
	size_A=float(area_A)/float(area_A+area_B);
	size_B=float(area_B)/float(area_A+area_B);
	rat_cut=float(psi_final)/(float(size_A)*float(size_B));
	fp_part<<"Benchmark name    : " <<f_without<<"\n";
	fp_part<<"Execution time       : "<<(double)(1.0*time.ru_utime.tv_sec+0.000001*time.ru_utime.tv_usec)<<"seconds"<<"\n";
	fp_part<<"Starting cut      : "<<psi_ini<<"\n";	
	fp_part<<"Final cut         : "<<psi_final<<"\n";
	fp_part<<"Percentage change : "<<red<<"%"<<"\n";
	fp_part<<"Ratio cut         : "<<rat_cut<<"\n";
	
   fp_plot.close();
   fp_part.close();	
		
}//end main


	//FUNCTIONS	
	long int cutset_change(long int i)
	{
	net_node* n_node;
	cell_node* c_node;
	long int d_psi=0;
	bool crit;
	char f_tp;
	n_node=Cell[i].head;
	while(n_node!=NULL)
	{
	c_node=n_node->NP->head;
	f_tp=c_node->CP->part;
	crit=0;
	while(c_node!=NULL)
	{
	if(c_node->CP->part != f_tp)
	crit=1;
	c_node=c_node->next;
	}
	if((crit==1) && (n_node->NP->critical==0))
	{
	d_psi++;
	n_node->NP->critical=1;
	}
	else if((crit==0) && (n_node->NP->critical==1))
	{
	d_psi--;
	n_node->NP->critical=0;
	}
	else if((crit==0) && (n_node->NP->critical==0)){ }
	else { }
	n_node=n_node->next;
	}
	return(d_psi);
	}
	
	void check_crit(long int rand3)
	{
	char f_tp;
	cell_node* c_node;
	net_node* n_node;
	bool crit;
	n_node=Cell[rand3].head;
	while(n_node!=NULL)
	{
	c_node=n_node->NP->head;
	f_tp=c_node->CP->part;
	crit=0;
	while(c_node!=NULL)
	{
	if(c_node->CP->part != f_tp)
	crit=1;
	c_node=c_node->next;
	}
	if((crit==1) && (n_node->NP->critical==0))
	{
	n_node->NP->critical=1;
	}
	else if((crit==0) && (n_node->NP->critical==1))
	{
	n_node->NP->critical=0;
	}
	else if((crit==0) && (n_node->NP->critical==0)) { }
	else {}
	n_node=n_node->next;
	}
	}

	void unmove()
	{
	long int f_i;
	for(f_i=0;f_i<=number_of_cells;f_i++)
	Cell[f_i].move=0;
	} 	
	
	void display()
	{
	int  f_j;
	for (f_j=0;f_j<number_of_modules;f_j++)
	fp_part<<Cell[f_j].Cell_name<<"\t"<<Cell[f_j].part<<"\n";
	for (f_j=1;f_j<=number_of_nets;f_j++)
	fp_part<<Net[f_j].Net_number<<"\t"<<Net[f_j].critical<<"\n";
	}

