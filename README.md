# Pac-Man Prog II — Projeto em C com Raylib

Este projeto é a implementação do trabalho prático de Prog II, usando somente C padrão + Raylib, seguindo o que foi visto em aula (arquivos, structs, ponteiros, alocação dinâmica, listas/vetores, etc.).

## Organização do projeto

- `src/`
  - `main.c`: ponto de entrada, inicializa Raylib e o `GameState`, controla o loop principal.
  - `game.h/.c`: estrutura `GameState`, carregamento de nível, controle de score, vidas, pellets, avanço de fase.
  - `map.h/.c`: leitura do mapa de arquivo texto (20x40), armazenamento dinâmico do mapa, posições iniciais de Pac-Man, fantasmas e portais.
  - `entity.h`: structs de posição (`Position`), direção (`Direction`), `Pacman` e `Ghost`.
  - `render.h/.c`: código de desenho usando Raylib (mapa, entidades, HUD, menu).
  - `menu.h/.c`: estado do menu (TAB) e ações (N, C, S, Q, V).
  - `save.h/.c`: funções de salvar/carregar jogo em arquivo binário.
- `assets/maps/`
  - `mapa1.txt`, `mapa2.txt`: mapas de teste 20x40 com paredes, pellets, power pellets, fantasmas e portais.

## Divisão de responsabilidades (Gus x Yas)

### Gus

- **Estruturas e estado do jogo**
  - Definir `GameState` (mapa, score, vidas, nível, timers, vetor dinâmico de fantasmas, etc.).
  - Garantir ausência de variáveis globais (estado sempre passado por ponteiro).
- **Mapas e metadados**
  - Implementar leitura dinâmica dos mapas (`map_load`, `map_free`).
  - Contar pellets e power pellets, inicializar `pelletsRemaining`.
  - Coletar posições iniciais de Pac-Man, fantasmas e portais.
- **HUD e controle de nível**
  - desenhar HUD (vidas, pontuação, nível, pellets restantes).
  - Avançar nível quando pellets chegam a zero (carregar `mapa2.txt`, etc.).
- **Persistência**
  - Implementar `save_game`/`load_game` com arquivo binário (estado completo do jogo).
  - Integrar com opções do menu (N, C, S, Q, V).

### Yas

- **Loop principal e entrada**
  - Implementar o loop do jogo em `main.c` / `game_update` usando Raylib (`WindowShouldClose`, `GetFrameTime`, etc.).
  - Tratar teclas de movimento (setas ou WASD) e tecla TAB para abrir/fechar menu.
- **Movimento e colisão**
  - Movimento discreto do Pac-Man (1 bloco por passo, 4 blocos/s).
  - Impedir movimento através de paredes; teleporte em portais.
  - Comer pellets/power pellets (atualizar score e `pelletsRemaining`).
- **Fantasmas**
  - Movimento simples dos fantasmas: seguir direção até interseção e, então, escolher nova direção válida de forma aleatória (sem reversão imediata).
  - Implementar modo vulnerável (8s) após power pellet; velocidade mais baixa; remoção do fantasma quando comido.
- **Renderização e menu**
  - Desenhar mapa e entidades com formas geométricas definidas no enunciado.
  - Desenhar o menu quando TAB estiver ativo, com as opções N/C/S/Q/V.

## Como compilar e executar — macOS (M1)

Pré-requisitos:

- `brew install raylib`
- `pkg-config` disponível (vem junto na instalação via Homebrew).

Comandos:

```bash
cd /Users/antonio/Documents/prog/pacman
cc src/*.c -o pacman $(pkg-config --cflags --libs raylib) \
  -framework CoreVideo -framework IOKit -framework Cocoa \
  -framework OpenGL -framework GLUT -framework CoreAudio
./pacman
```

### Explicando o comando de compilação

- `cd /Users/antonio/Documents/prog/pacman`  
  Entra na pasta do projeto, onde estão `src/*.c` e `assets/`.

- `cc src/*.c -o pacman`  
  - `cc`: compilador C padrão (no macOS é o `clang`).  
  - `src/*.c`: todos os arquivos `.c` dentro da pasta `src/` (main.c, game.c, map.c, etc.).  
  - `-o pacman`: nome do executável gerado será `pacman`.

- `$(pkg-config --cflags --libs raylib)`  
  - `pkg-config`: ferramenta que sabe onde estão os diretórios de include e as bibliotecas instaladas.  
  - `--cflags raylib`: imprime as flags de compilação, por exemplo:  
    `-I/opt/homebrew/Cellar/raylib/5.5/include`  
  - `--libs raylib`: imprime as flags de link, por exemplo:  
    `-L/opt/homebrew/Cellar/raylib/5.5/lib -lraylib`  
  - `$( ... )`: faz o shell executar o comando dentro dos parênteses e substituir pelo resultado.  
  - Resultado típico completo:  
    `-I/opt/homebrew/Cellar/raylib/5.5/include -L/opt/homebrew/Cellar/raylib/5.5/lib -lraylib`

- `-framework CoreVideo`  
  Liga com o framework CoreVideo da Apple (controle de tempo/frames e integração com vídeo).

- `-framework IOKit`  
  Liga com o framework IOKit (acesso a dispositivos de entrada/saída, como teclado e mouse, usado indiretamente pela Raylib).

- `-framework Cocoa`  
  Framework de janelas do macOS (criação de janela, eventos da aplicação, etc.).

- `-framework OpenGL`  
  Biblioteca gráfica para renderização 2D/3D; Raylib usa OpenGL internamente para desenhar os elementos.

- `-framework GLUT`  
  Toolkit utilitário para OpenGL, com funções para janelas/entrada compatíveis com o backend da Raylib.

- `-framework CoreAudio`  
  Framework de áudio do macOS, usado pela Raylib para sons e música.

Resumo:

- `src/*.c + -o pacman` → compila todo o código C do projeto em um único executável.  
- `$(pkg-config ...)` → diz ao compilador onde estão os headers e as bibliotecas da Raylib e linka com `-lraylib`.  
- `-framework ...` → bibliotecas do macOS necessárias para a Raylib abrir a janela, desenhar na tela e tocar sons.

### Executar o jogo

```bash
./pacman
```

Executa o binário compilado. A janela da Raylib é aberta e o loop do jogo roda com base nas funções implementadas em `src/`.

- **Atalho útil:** pressione `F` a qualquer momento para alternar entre janela e tela cheia. Ao sair do fullscreen, a janela volta para o tamanho original (1600x840).

## Como compilar e executar — Windows (MSYS2 + Raylib)

Passos baseados na especificação do professor, usando MSYS2 no Windows.

### 1. Instalar MSYS2 e pacotes

1. Baixar e instalar MSYS2: <https://www.msys2.org/>  
2. Abrir o terminal **MSYS2 UCRT64**.  
3. Instalar GCC e Raylib:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc
pacman -S mingw-w64-ucrt-x86_64-raylib
```

### 2. Clonar/copiar o projeto

- Colocar a pasta `pacman` em algum diretório acessível pelo MSYS2, por exemplo:  
  `C:/msys64/home/<usuario>/pacman`

### 3. Compilar

No terminal **MSYS2 UCRT64**, entrar na pasta do projeto:

```bash
cd ~/pacman
gcc src/*.c -o pacman.exe -lraylib -lwinmm -lgdi32 -lopengl32
```

Explicando as libs extras:

- `-lraylib` → biblioteca principal da Raylib (Windows).  
- `-lwinmm` → multimídia do Windows (som, temporização).  
- `-lgdi32` → interface gráfica (desenho em janelas).  
- `-lopengl32` → implementação de OpenGL do Windows (renderização).

### 4. Executar

Ainda no mesmo terminal:

```bash
./pacman.exe
```

A janela do jogo abre no Windows. A lógica de código é a mesma para macOS e Windows; só muda a forma de instalar Raylib e as flags de compilação/linkedição.
