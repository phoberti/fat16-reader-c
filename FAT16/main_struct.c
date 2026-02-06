#include <stdio.h>
#include <stdlib.h>


//struct para o setor de boot da FAT16. Struct pronta do exemplo "main_struct.c" disponibilizado no teams
typedef struct fat_BS
{
	unsigned char 		bootjmp[3];
	unsigned char 		oem_name[8];
	unsigned short 	    bytes_per_sector;
	unsigned char		sectors_per_cluster;
	unsigned short		reserved_sector_count;
	unsigned char		table_count;
	unsigned short		root_entry_count;
	unsigned short		total_sectors_16;
	unsigned char		media_type;
	unsigned short		table_size_16;
	unsigned short		sectors_per_track;
	unsigned short		head_side_count;
	unsigned int 		hidden_sector_count;
	unsigned int 		total_sectors_32;
 
	//this will be cast to it's specific type once the driver actually knows what type of FAT this is.
	unsigned char		extended_section[54];
 
}__attribute__((packed)) fat_BS_t;

//struct para as entradas de diretorio
typedef struct informacao_arquivo
{
	unsigned char nome[8];
	unsigned char extensao[3];
	unsigned char formato_do_arquivo;
    int tamanho;
	int primeiro_cluster;
}__attribute__((packed)) informacao_arquivo_t;

void print_arq(informacao_arquivo_t entry){
	for( int i=0; i<8; i++){//para o nome
		if(entry.nome[i] == ' ') break;// sair espacos
		printf("%c", entry.nome[i]);
	}
	if(entry.formato_do_arquivo == 0x20) printf("."); // printa um ponto para a extensao caso for arquivo

	for(int i=0; i<3; i++){// para extensao
		if(entry.extensao[i] == ' ')break;//espacos
		printf("%c", entry.extensao[i]);
	}
	printf(" - ");

	if(entry.formato_do_arquivo == 0x10){
		printf(" Diretorio - ");
	}else if(entry.formato_do_arquivo == 0x20){
		printf(" Arquivo - ");
	}
	
	printf("First Cluster: %d - tamanho: %d bytes\n", entry.primeiro_cluster, entry.tamanho);

}
int verifica(informacao_arquivo_t entry, char *escolha){//compara o numero de entrada com o nome do arquivo

char fname[12];
int tam = 0;

for(int i=0; i<8; i++){ //para o nome

if(entry.nome[i] == ' ') break;//se tiver espaco
fname[i] = entry.nome[i];
tam++;

}

if(entry.formato_do_arquivo == 0x20){ // se for um arquivo adciona um ponto no vet para a extensao
	fname[tam] = '.';
	tam++;
}

for(int i=0; i<3; i++){ //para a extensao
	if(entry.extensao[i] == ' ') break;
	fname[tam] = entry.extensao[i];
	tam++;
}

for(int i=0; i<tam; i++){ // comparacao final
	if(fname[i]!= escolha[i]) return 0;
}
return 1;
}

int main()
{

    FILE *fp; // ponteiro para o arquivo
    fat_BS_t  boot_record; // struct para o setor de boot
	char nome_arquivo[80], escolha[20]; // array para o nome e escolha do arquivo
	unsigned short *fat; // ponteiro para a tabela de alocação de arquivos 
	unsigned char *diretorio_raiz, entrada[32]; // Ponteiro para o diretório raiz e uma entrada
	unsigned int root, dados; // Variáveis para o inicio do diretorio raiz e da area de dados
	int setores_root_dir, tamanho, cluster, contador_entradas=0, posicao=0, procura=0, bytes_por_cluster, num; // variaveis para controle


	printf("Digite o nome do arquivo: ");
	scanf("%s",nome_arquivo);

	if((fp = fopen(nome_arquivo, "rb")) == NULL){
		printf("\nArquivo nao pode ser aberto");
		return 0;
	}

	//le o arquivo para puxar as informacoes
    fseek(fp, 0, SEEK_SET);
    fread(&boot_record, sizeof(fat_BS_t),1, fp);

	// calculo do numero de setores para o diretorio raiz
	setores_root_dir = boot_record.root_entry_count * 32 / boot_record.bytes_per_sector;

	// calculo da posicao dos setores do diretorio raiz
	root = boot_record.reserved_sector_count + (boot_record.table_count * boot_record.table_size_16);

	// calculo dos setores da area de dados
	dados = root + (boot_record.root_entry_count * 32 / boot_record.bytes_per_sector);

	// alocacao de memoria para o diretorio raiz
	informacao_arquivo_t *arquivos = malloc(sizeof(informacao_arquivo_t) * setores_root_dir);

	// variavel temporaria para as informacoes do arquivo
	informacao_arquivo_t file;

	printf("\nSO - FAT  - PEDRO BERTI\n");
	printf("----------------------------\n");
	printf("a) Numero de FATs: %x\n", boot_record.table_count);
	printf("b) Posicao inicial de cada FAT no disco:\n");
for (int i = 0; i < boot_record.table_count; i++) {
    unsigned int pos_fat = boot_record.reserved_sector_count + (i * boot_record.table_size_16);
    printf("FAT[%d]: Setor %u\n", i + 1, pos_fat);
}
	printf("c) A posicao inicial do setor de diretorio raiz: %u\n", root);
	printf("d) A posicao inicial da area de dados: %u\n", dados);
	printf("e) Arquivos e diretorios armazenados na raiz: \n");

	//alocacao para a fat
	fat = malloc(boot_record.table_size_16 * boot_record.bytes_per_sector);

	//coloca o ponteiro no inicio da FAT
	fseek(fp, boot_record.reserved_sector_count * boot_record.bytes_per_sector, SEEK_SET);
	fread(fat, boot_record.table_size_16 * boot_record.bytes_per_sector, 1, fp);

	//alocacao para o root dir
	diretorio_raiz = malloc(boot_record.root_entry_count * 32);

	//coloca o ponteiro no inicio do root dir
	fseek(fp, boot_record.reserved_sector_count * boot_record.bytes_per_sector + (boot_record.table_count * boot_record.table_size_16 * boot_record.bytes_per_sector), SEEK_SET);
	fread(diretorio_raiz, setores_root_dir * 32, 1, fp);

	while(contador_entradas < boot_record.root_entry_count){//percorre as entradas do root dir
		if(diretorio_raiz[contador_entradas * 32] == 0x00) break; // verifica se a entrada e vazia

		if(diretorio_raiz[contador_entradas * 32] == 0xE5){//verifica se a entrada ta marcada como excluida
		contador_entradas++;
		continue;
		}

		if(diretorio_raiz[(contador_entradas * 32) + 11] == 0x0F){//verifica se o valor e um longfilename
		contador_entradas++;
		continue;
		}

		for(int i=0; i<32; i++){//copia dos 32 bytes do root dir
			entrada[i] = diretorio_raiz[i + (contador_entradas * 32)];
		}

		//verifica se e um diretorio ou arquivo 10=dir 20=arquivo
		if(entrada[11] == 0x10 || entrada[11] == 0x20){
			//pega o tamanho do arquivo em bytes
			tamanho = entrada[31] << 24 | entrada[30] << 16 | entrada[29] << 8 | entrada[28];
			// pega o numero do primeiro cluster
			cluster = entrada[27] << 8 | entrada[26];
		}

		for(int i=0; i<8; i++){//pega o nome
			file.nome[i] = entrada[i];
		}

		for(int i=0; i<3; i++){//pega a extensao
			file.extensao[i] = entrada[i+8];
		}

		file.formato_do_arquivo =  entrada[11]; // salva se eh um arquivo ou um diretorio
		file.tamanho = tamanho; // salva o tamanho
		file.primeiro_cluster = cluster; //salva o primeiro cluster
		arquivos[posicao] = file; // salva os dados da struct no vetor
		posicao++; //proxima posicao do vetor
		contador_entradas++; // proxima entrada
	}

	for(int i=0; i<posicao; i++){
		printf("[%d] - ", i);
		print_arq(arquivos[i]);
	}

	printf("\nDados do Boot Record:\n");
	printf("Bytes por Setor: %hd\n", boot_record.bytes_per_sector);
	printf("Setores Reservados: %hd\n", boot_record.reserved_sector_count);
	printf("Setores por Cluster: %x\n", boot_record.sectors_per_cluster);
	printf("Numero de FATs %x\n", boot_record.table_count);
	printf("Setores por FAT: %hd\n", boot_record.table_size_16);
	printf("Entradas do Root Dir: %hd\n", boot_record.root_entry_count);

	while(procura == 0){
		printf("\nDigite o numero do arquivo que deseja abrir: ");
		scanf("%s", escolha);
		int index = atoi(escolha); // converte para um numero inteiro
		if(index>=0 && index<posicao){ // ve se é valido
		// se for valido, seleciona o arquivo pelo indice
			procura = 1;
			posicao = index;
		}else{
			for (int i = 0; i < posicao; i++)
			{
				if(verifica(arquivos[i], escolha)){//verficacao do nome do arquivo com o valor da entrada
				// se a entrada do usuario for valida, seleciona pelo indice
				procura=1;
				posicao=i;
				}
			}
			
		}
	}

		printf("\n---\n");
		printf("\nEscolha: ");
		file = arquivos[posicao];
		print_arq(file);

// calculo do numero do cluster para salvar o arquivo
num = (file.tamanho / boot_record.bytes_per_sector) + 1;
// guarda os unmeros de clusters que o arquivo usa
int file_clusters[num];
int clusters = 1;
file_clusters[0] = file.primeiro_cluster;

for(int i=0; i<num; i++){
// mapeia os clusters do arquivo com a tabela fat
	file_clusters[i + 1] = fat[file_clusters[i]];
//verifica o valor da tabela
if(file_clusters[i + 1] >= 0xFFFF) break;

clusters++;
}
 
//calculo do tamanho em bytes de um cluster
bytes_por_cluster = boot_record.sectors_per_cluster * boot_record.bytes_per_sector;
if(file.formato_do_arquivo == 0x10){
	unsigned char dados2[bytes_por_cluster];

	//pega o inicio do cluster
fseek(fp, (dados + ((file_clusters[0] - 2 ) * boot_record.sectors_per_cluster)) * boot_record.bytes_per_sector, SEEK_SET);
fread(&dados2, bytes_por_cluster, 1, fp);

informacao_arquivo_t dir_file;
contador_entradas = 0;

//para acessar o subdiretório
while(contador_entradas < bytes_por_cluster / 32){
	if(dados2[contador_entradas *32] == 0x00) break;//verifica se é vazio

	if(dados2[contador_entradas *32] == 0xE5){ //verifica se ta marcado como excluido
	contador_entradas++;
	continue;
	}

	if(dados2[(contador_entradas * 32) + 11] == 0x0F){ // verifica se é longfilename
		contador_entradas++;
		continue;
	}

	for(int i=0; i<32; i++){//armazena os dados da entrada
		entrada[i] = dados2[ i + (contador_entradas * 32)];
	}

	//verifica se é um diretório (10) ou arquivo (20)
	if(entrada[11] == 0x10 || entrada[11] == 0x20){
	//pega o tamanho do arquivo em bytes
	tamanho = entrada[31] << 24 | entrada[30] << 16 | entrada[29] << 8 | entrada[28];
	//pega o numero do primeiro cluster
	cluster = entrada[27] << 8 | entrada[26]; 
	}

	for(int i=0; i<8; i++){//pega o nome
		dir_file.nome[i] = entrada[i];
	}

	for(int i=0; i<3; i++){ // pega a extensao
		dir_file.extensao[i] = entrada[i+8];
			}

dir_file.formato_do_arquivo=entrada[11];
dir_file.tamanho=tamanho;
dir_file.primeiro_cluster=cluster;
print_arq(dir_file);
contador_entradas++;

}
}else if(file.formato_do_arquivo == 0x20)//caso seja um arquivo
{
	unsigned char dados3[file.tamanho];
	//clusters do arquivo
	for (int i=0; i<clusters; i++){
		//pega o inicio do cluster
		fseek(fp, (dados + ((file_clusters[i] -2)*boot_record.sectors_per_cluster))*boot_record.bytes_per_sector, SEEK_SET);
		//calculo do numero de bytes lidos
		int num_bytes = i * bytes_por_cluster;

		if (i == clusters - 1)
		{//verifica se é o ultimo cluster
			fread(&dados3[num_bytes], file.tamanho - (num_bytes), 1, fp);
		}
		else // se for o ultimo cluster le o resto dos bytes
		fread(&dados3[num_bytes], bytes_por_cluster, 1, fp);
	}
	printf("\n");
	for(int i=0; i<file.tamanho; i++){
		printf("%c", dados3[i]);
	}
}

free(diretorio_raiz);
free(fat);
fclose(fp);

    return 0;
}