# Pac-Man Prog II — Cards por pessoa

Organização tipo Jira: cada card tem id, status inicial `To Do`, responsável (Gus ou Yas) e breve descrição. Quando forem avançando, troquem status para `In Progress` ou `Done` e anotem data.

## Cards do Gus

- **[G1] To Do — Estrutura e estado do jogo**  
  Definir `GameState` com mapas carregados, pontuação, vidas, nível atual, temporizadores, vetores dinâmicos (pellets restantes, fantasmas vivos). Sem variáveis globais.
- **[G2] To Do — Leitura dinâmica de mapas**  
  Carregar `mapa1.txt`, `mapa2.txt`... (20x40) em memória dinâmica, contar pellets/power, localizar portais, pacman inicial e fantasmas iniciais.
- **[G3] To Do — Portais e metadados**  
  Mapear pares de portais por linha/coluna, manter função utilitária para teleporte mantendo direção.
- **[G4] To Do — HUD**  
  Desenhar HUD 40px: vidas, score (6 dígitos), nível, pellets restantes; opcional legenda.
- **[G5] To Do — Persistência (save/load)**  
  Arquivo binário com estado completo: nível atual, mapa corrente (pellets já comidos), posição/direção de pacman e fantasmas, timers, score, vidas. Funções `salvarJogo(GameState*)` e `carregarJogo(GameState*)`.
- **[G6] To Do — Controle de nível**  
  Avançar para próximo mapa ao zerar pellets; reset de timers/movimentos; reuso de `GameState` sem vazamentos.
- **[G7] To Do — Integração de menu**  
  Handlers para opções do menu (N/C/S/Q/V) chamando init/reset/save/load.
- **[G8] To Do — Documentação interna**  
  Comentários curtos nas partes não óbvias (alocação, lifecycle), diagrama simples de structs para relatório.

## Cards da Yas

- **[Y1] To Do — Loop principal e input**  
  `InitWindow`, `SetTargetFPS(60)`, laço central, leitura de setas/WASD, controle de pausa/menu com TAB.
- **[Y2] To Do — Movimento do Pac-Man**  
  Passo discreto 1 bloco, velocidade 4 blocos/s; bloqueio em paredes; consumo de pellet/power (pontuação e pellets restantes); aplicação de portais.
- **[Y3] To Do — Fantasmas (movimento base)**  
  Estrutura por fantasma (pos, dir, vulnerável flag, timer). Movimento contínuo até interseção; nova direção aleatória válida sem reversão imediata; bloqueio em parede/outro fantasma (pode relaxar colisão entre eles).
- **[Y4] To Do — Modo vulnerável**  
  Ativar 8s ao comer power pellet; cor branca; velocidade 3 blocos/s; remoção do fantasma quando comido (score +100).
- **[Y5] To Do — Colisões e vidas**  
  Detectar Pac-Man x fantasma (não vulnerável: -1 vida, score -200 min 0, respawn); game over; vitória ao zerar pellets.
- **[Y6] To Do — Renderização**  
  Desenho conforme enunciado: parede azul, pellet círculo branco, power pellet círculo verde 40px, pacman círculo amarelo, fantasma círculo vermelho/branco, portal quadrado rosa. Área jogável 1600x800 + HUD.
- **[Y7] To Do — Menu visual**  
  Overlay simples quando TAB: opções N/C/S/Q/V com highlight da seleção; retorno ao jogo.
- **[Y8] To Do — Testes locais**  
  Rodar cenários de movimento, portais, power pellet, troca de nível; anotar casos de borda para apresentação.

## Cards de apoio (qualquer um dos dois)

- **[A1] To Do — Mapas de exemplo**  
  Criar `assets/maps/mapa1.txt` e `mapa2.txt` minimamente jogáveis, com portais e power pellets.
- **[A2] To Do — Build/exec scripts**  
  Makefile ou script `build.sh` para compilar tudo com `pkg-config --cflags --libs raylib` e frameworks do macOS.
- **[A3] To Do — README e Relatório**  
  Como compilar/rodar/salvar/carregar, divisão de tarefas (cards), decisões principais. README separado do relatório.
- **[A4] To Do — Check final**  
  Passar checklist dos requisitos mínimos antes da entrega; remover binários; zipar com nomes do grupo.

## Dicas rápidas de fluxo
- Começar por G1/G2 e Y1/Y2 em paralelo. Depois integrar HUD (G4) com render (Y6).
- Antes de Save/Load (G5), garantir que `GameState` cobre tudo que muda no jogo.
- Para testes, usar velocidade fixa e logs no terminal (`printf`) nos eventos-chave (pegar power, perder vida, teleporte).
