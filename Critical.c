#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include<time.h>
#include<sys/time.h> 

#define LIVECELL 1
#define DEADCELL 0
#define NUM_GENERATIONS 1000
#define SIZE 2048
#define SRAND_VALUE 1985
#define MAX_THREADS 8

// FUNÇÕES
void initializeGrid(int** grid);
int** allocateMemory();
int update(int** grid, int x, int y);
void copyMatrix(int** grid, int** newgrid);
void printMatrix(int** grid);
void countCells(int** grid);
int getNeighbors(int** grid,int x, int y);
void clearGame(int **grid);
void getSpace(int** grid);

// VARIÁVEL GLOBAL
int num_cells = 0; 

int main(int argc, char *argv[])
{
    int i, j, k;
    double startTime, endTime, executionTime;
    int** newgrid;
    int** grid; 

    srand(SRAND_VALUE); 

    /*
    É FEITA A ALOCAÇÃO DE MEMÓRIA PARA OS TABULEIROS (GERAÇÃO ATUAL E PRÓXIMA GERAÇÃO).
    O TABULEIRO ATUAL É INICIALIZADO COM ZEROS E UNS. ZEROS REPRESENTAM CÉLULAS MORTAS E UNS
    REPRESENTAM CÉLULAS VIVAS. O TABULEIRO QUE REPRESENTA A PRÓXIMA GERAÇÃO INICIALMENTE É
    PRENCHIDO INTEGRALMENTE COM VALORES ZEROS.
    */
    grid = allocateMemory();
    newgrid = allocateMemory(); 
    initializeGrid(grid);
    clearGame(newgrid); 

    /*
    NESTE TRECHO DE CÓDIGO, O JOGO É EXECUTADO PELO NÚMERO DE GERAÇÕES DEFINIDO INICIALMENTE.
    AQUI UTILIZAMOS A DIRETIVA PRAGMA OMP PARALLEL FOR PARA DIVIDIR O LAÇO INTERNO (COLUNAS)
    ENTRE AS THREADS.
    */
    for(k = 0; k < NUM_GENERATIONS; k++) 
    {   
        for(i = 0; i < SIZE; i++)
        {   
            #pragma omp parallel num_threads(MAX_THREADS)
            #pragma omp for private(j)
            for( j = 0; j < SIZE; j++ )
            {
                /*
                CADA CÉLULA DO TABULEIRO TEM SEU ESTADO ATUALIZADO PARA A PRÓXIMA
                GERAÇÃO DE ACORDO COM O SEU NÚMERO DE VIZINHOS.
                */
                newgrid[i][j] = update(grid,i, j);
            }
                    
        }
        /*
        O TABULEIRO REPRESENTANTE DA PRÓXIMA GERAÇÃO TEM SEUS VALORES COPIADOS PARA O
        TABULEIRO ATUAL PARA QUE UMA NOVA GERAÇÃO POSSA SER SIMULADA.
        */
        copyMatrix(grid, newgrid);   
    }
    
    // CHAMADA DO MÉTODO PARA CONTAGEM DAS CÉLULAS VIVAS

    startTime = omp_get_wtime(); 
    countCells(grid); 
    endTime = omp_get_wtime();
    
    printf("\n\n NÚMERO DE CÉLULAS VIVAS: %d ", num_cells);  

    executionTime = endTime - startTime;
    printf("\n\n TEMPO DE EXECUÇÃO, EM SEGUNDOS, DO TRECHO DE CONTAGEM DAS CÉLULAS VIVAS: %f  ", executionTime);

    getSpace(grid); 
    getSpace(newgrid);

    return 0;
}

//        ****************  MÉTODO PARA CONTAGEM DAS CÉLULAS VIVAS  ****************            //

void countCells(int** grid) { 
    int i, j;  
    for( i = 0; i < SIZE; i++) {
        #pragma omp parallel num_threads(MAX_THREADS)
        #pragma omp for private(j)
        for( j = 0; j < SIZE; j++ ) {
            if( grid[i][j] == 1 ) {
                #pragma omp critical (CountCells)
                num_cells = num_cells + 1;
            }
        }
    }
}

/*
O MÉTODO initializeGrid É USADO PARA INICIALIZAR O GRID COM OS VALORES DE ZEROS E UNS
*/
void initializeGrid(int **grid){
    int i, j;
    for(i = 0; i < SIZE; i++) {
        for(j = 0; j < SIZE; j++) {
            grid[i][j] = rand() % 2;
        }
    }
}

/*
O MÉTODO allocateMemory FAZ A ALOCAÇÃO DE MEMÓRIA DE ACORDO COM O TAMANHO DEFINIDO PARA O GRID
*/
int** allocateMemory(){ 

    int i; 
    int **grid = (int**)malloc(SIZE*sizeof(int*));
    for(i = 0; i < SIZE; i++) { 
        grid[i] = (int*) malloc(SIZE*sizeof(int));
    }
    return (grid);
} 

/*
O MÉTODO clearGame FAZ COM QUE TODAS AS CÉLULAS DO GRID RECEBAM O VALOR ZERO
*/
void clearGame(int **grid) {
    
    int i, j;
    for(i = 0; i < SIZE; i++){
        for(j = 0; j < SIZE; j++){
            grid[i][j] = 0;
        }
    }
}

/*
O MÉTODO update DEFINE O PRÓXIMO ESTADO DE CADA CÉLULA DO TABULEIRO DE ACORDO COM AS REGRAS
DO JOGO, OU SEJA, SEU NÚMERO DE VIZINHOS.
*/
int update(int** grid,int x, int y)
{

    int num_neighbors;
    num_neighbors = getNeighbors(grid, x, y); 

    if( num_neighbors < 2 || num_neighbors > 3) 
	return DEADCELL;
    else if( ( grid[x][y] == 1 ) && ( num_neighbors == 2 || num_neighbors == 3) )
	return LIVECELL;
    else if( grid[x][y] == 0 && num_neighbors == 3)
	return LIVECELL;
    else return DEADCELL;
}

/*
O MÉTODO copyMatrix É USADO PARA FAZER A TROCA ENTRE OS TABULEIROS.
*/
void copyMatrix(int** grid, int** newgrid) 
{
    int i, j;
    for(i = 0; i < SIZE; i++){
	    for(j = 0; j < SIZE; j++){
            grid[i][j] = newgrid[i][j];
	    }   
    }
}

/*
O MÉTODO printMatrix É USADO PARA FAZER A IMPRESSÃO DO TABULEIRO. UTILIZADA APENAS PARA TESTES.
*/
void printMatrix(int** grid){
    int i, j;
    for( i = 0; i < SIZE; i++) {
        for( j = 0; j < SIZE; j++ ) {
            printf("%d ",grid[i][j]);  
        }
        printf("\n");
    }
}

/*
O MÉTODO getNeighbors FAZ A CONTAGEM DO NÚMERO DE VIZINHOS DE CADA UMA DAS CÉLULAS DO 
TABULEIRO.
*/
int getNeighbors(int** grid,int x, int y) {

    int alive_neighbours = 0;

    int left = grid[x][((y-1+SIZE)%SIZE)];
    int right = grid[x][((y+1+SIZE)%SIZE)];
    int top = grid[(((x-1+SIZE)%SIZE))][y];
    int bottom = grid[((x+1+SIZE)%SIZE)][y];
    int topleft = grid[((x-1+SIZE)%SIZE)][((y-1+SIZE)%SIZE)];
    int topright = grid[(((x-1+SIZE)%SIZE))][((y+1+SIZE)%SIZE)];
    int bottomleft = grid[((x+1+SIZE)%SIZE)][((y-1+SIZE)%SIZE)];
    int bottomright = grid[((x+1+SIZE)%SIZE)][((y+1+SIZE)%SIZE)];	 

    alive_neighbours = left+right+top+bottom+topleft+topright+bottomleft+bottomright;

    return alive_neighbours;
}

/*
O MÉTODO getSpace FAZ A LIBERAÇÃO DE MEMÓRIA ALOCADA PARA OS TABULEIROS.
*/
void getSpace(int** grid) { 
    int i = 0;
    for ( i = 0; i < SIZE; i++){
        free(grid[i]);
    }
    free(grid);
}
