/***********************************************************
***** NAME	:	HAVISH NALLAPAREDDY                      ***
***** DATE	:	April 22nd, 2020                         ***
***** COURSE:	CSCE 5580 COMPUTER NETWORKS              ***
************************************************************
** In this project, I  wrote a complete C program to     ***
** implement the OSPF link-state (LS) algorithm for      ***
** a given set of nodes       							 ***
***********************************************************/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define SELF_DISTANCE 8888
#define MAX 50
 
void OSPF(int graph[MAX][MAX],int n,int begin,char first);
 
int main()
{
    int graph[MAX][MAX],i,j,n,u;
    char file_name[20],first_node,source_node,node_count;
    
    printf("OSPF Link-State (LS) Routing:\n");
    printf("-----------------------------\n");
    printf("Enter the number of routers:");
    scanf("%d",&n);
    if(n>26){
    	printf("please enter the rounters count in the range of 26\n");
    	exit(-1);
	}
    printf("Enter filename with cost matrix values: ");
    scanf("%s",file_name);
    getchar();
    
        
	// reading the file with given input name
	FILE* in_file = fopen(file_name, "r");  
    u = source_node - first_node;
    //opening the input file with given file name
	if(in_file==NULL)
	{
		printf("error: unable to open file: %s \n",file_name);
		exit(-1);
	}
	
	//assigning the values of the file array to matrix
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			
			fscanf(in_file,"%d",&graph[i][j]);
			
			if(graph[i][j]<0)
			   graph[i][j]=100000;
			node_count++;
		}
	}
	if(node_count<n){
		printf("File : %s has insufficient data",file_name);
		exit(-1);
	}
	printf("Enter character representation of first node: ");
    scanf("%c",&first_node);
    getchar();
    if((int)(first_node)+n-1 > 122){
    	printf("Enter the character representation first node in %c to %c range",'a',123-n);
    	exit(-1);
	}
    printf("Enter the source router: ");
    scanf("%c",&source_node);
    
    if((int)source_node > (int)first_node+n-1){
    	printf("Enter the source router in the range of %c to %c ",first_node,first_node+n-1);
    	exit(-1);
	}
	
    u = source_node - first_node; 
    //shortest path algorithm implementation
    OSPF(graph,n,u,first_node);
    
    return 0;
}

void OSPF(int graph[MAX][MAX],int n,int begin,char first)
{
	// initialising values 
    int path_cost[MAX][MAX],distance[MAX],previous[MAX];
    int seen[MAX],count,min_till_now,nextnode,i,j,k;
    
    //previous[] stores the previous node of each node
    //create the path_cost matrix
    for(i=0;i<n;i++){
    	for(j=0;j<n;j++){
    		if(graph[i][j]==0)
                path_cost[i][j]=SELF_DISTANCE;
            else
                path_cost[i][j]=graph[i][j];
		}
            
	}
    	
    //initialize previous[],distance[] and seen[]
    for(i=0;i<n;i++)
    {
        distance[i]=path_cost[begin][i];
        previous[i]=begin;
        seen[i]=0;
    }
    
    distance[begin]=0;
    seen[begin]=1;
    count=1;
    
    while(count<n-1)
    {
        min_till_now=SELF_DISTANCE;
        
        //nextnode gives the node at minimum distance
        for(i=0;i<n;i++){
        	if(distance[i]<min_till_now&&!seen[i])
            {
                min_till_now=distance[i];
                nextnode=i;
            }
		}
        //see the nextnode has a minimum distanace than current 
        seen[nextnode]=1;
        for(i=0;i<n;i++){
          	if(!seen[i]){
           		if(min_till_now+path_cost[nextnode][i]<distance[i])
                {
                    distance[i]=min_till_now+path_cost[nextnode][i];
                    previous[i]=nextnode;
                }
			}
                    
		}        
        count++;
    }
 
    //print the path and distance of each node
    for(i=0;i<n;i++)
        {
        	printf("\n%c ==> %c",first+begin,first+i);
            printf("\npath cost : %d",distance[i]);
            printf("\npath taken : ");
            int path_cnt = 0;
            char path[20];
            for(j=i;j!=begin;){
            	path[path_cnt++] = first+previous[j];
            	j = previous[j];
			}
			for( k = path_cnt - 1 ;k>=0;k--){
				printf("%c --> ",path[k]);
			}
			printf("%c ",first+i);
      }
      printf("\n");
}
    
