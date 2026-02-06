# Leitor FAT16 em C

Projeto desenvolvido em C para leitura e navegação básica em uma imagem de sistema de arquivos **FAT16**.

O programa:
- Lê o **Boot Sector** (BPB) e extrai parâmetros como bytes por setor, setores por cluster, número de FATs, etc.
- Calcula a posição do **Root Directory** e da **área de dados**
- Lê a **FAT** e o **diretório raiz**, listando arquivos e diretórios
- Permite selecionar uma entrada por **índice** ou pelo **nome**
- Se a entrada for:
  - **diretório**: lista as entradas do subdiretório (lendo o cluster correspondente)
  - **arquivo**: lê os clusters do arquivo via FAT e imprime o conteúdo na tela

## Entrada

O programa solicita no terminal o nome do arquivo/imagem a ser aberto:

- Exemplo: `fat16.img` (imagem binária de um volume FAT16)

Observação: o arquivo deve estar no mesmo diretório do executável ou você deve informar o caminho completo.

## Saída

- Informações do Boot Record (BPB)
- Posição inicial:
  - de cada FAT
  - do Root Directory
  - da área de dados
- Lista de arquivos e diretórios presentes na raiz
- Conteúdo do arquivo escolhido (se for arquivo) ou listagem do subdiretório (se for diretório)

## Como compilar e executar

### Linux/macOS (gcc)

- gcc -O2 -Wall -Wextra -std=c11 -o fat16 src/main.c

- ./fat16

### Windows (MinGW)

- gcc -O2 -Wall -Wextra -std=c11 -o fat16.exe src/main.c
- fat16.exe


Ao rodar, digite o nome da imagem:

- Digite o nome do arquivo: fat16.img
