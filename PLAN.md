# COQUÍ — plano do firmware (nome provisório)

Groovebox de ritmos latinos e caribenhos para o M-VAVE FM-1. Fork do
[GR!MM](https://github.com/GRiMMBIENT/grimm) (GPL-3.0), que roda na plataforma Felucca → SLOOP → Dinghy.

## Decisões

| Tema | Decisão |
| --- | --- |
| Mood | Salsa e bomba/plena de raiz, com um toque de reggaetón das antigas. Clima: rua de San Juan à noite |
| Gêneros da 1.0 | Dembow, salsa (clave 2-3 / 3-2), bomba sicá, plena, merengue, cumbia |
| Percussão | 100 % sintetizada (só inteiros). Os 4 slots de sample do GR!MM ficam livres para o usuário |
| Formato | 4 tracks: T1 e T2 percussão, T3 baixo tumbao, T4 harmonia (piano montuno / metais) |
| Base | Fork do GR!MM 2.0.2 |
| Visual | Paleta neon tropical (a definir com o usuário) |
| Hardware | Testes no computador e no emulador web até haver um FM-1 físico. Aceitar as duas flashes (`0x856014` e `0xB36014`) |
| Repositório | `~/dev/pessoal/coqui-fm1`, git local. Publicar no GitHub só quando o usuário mandar |

## Fases

0. **Ambiente:** toolchain JieLi + SDK AC79. Compilar o GR!MM original e rodar os testes host.
1. ✅ **Identidade (feita 2026-11-05):** `FM-1_923` (livre no catálogo do Hub, `thefirmwarehub.com/catalog.json`),
   `APP_NAME "COQUI"`, pacote `coqui.fwsc`, versão 0.1. Magics próprios: `CQOB`, `CQSL`, `CQST`, `CQSP` e sons `C`
   (os saves do GR!MM não são lidos). Flash `0x856014` e `0xB36014` aceitas no app, no rescue e no loader
   (`FL_JEDEC_OK`). Paleta noche tropical (ROSA, MANGO, MAR, LIMA) e boot com lua, palmeira, cucubanos, mar e o
   coquí cantando. O teste de compatibilidade com o GR!MM 1.0 saiu. Plano original:
    nome, identidade `FM-1_9NN` livre (conferir no Hub), `STORE_MAGIC` próprio, `APP_NAME`,
   tela de boot, paleta, créditos em `LICENSING.md`.
2. **Motor de percussão:** conga (quinto/conga/tumba: aberto, abafado, slap), bongô, timbal, cowbell, clave,
   güiro, maracas/shaker, cajón, kick e caixa de reggaetón. Custo por voz medido com `fm1_ticks()`.
3. ✅ **Padrões (feita 2026-11-05):** GROOVE (`firmware/src/groove.c`), página nova no EDIT dos tracks PERC: 14 estilos
   (DEMBOW, PERREO, SALSA 2-3/3-2, SON, MAMBO, CHACHA, BOMBA SICA/YUBA 6/8, PLENA, MERENGUE, BACHATA, CUMBIA, SOCA) de
   2 compassos com acentos e ghosts; PARTS (KIT, MANOS, CAMPANA), FILL a cada 2/4/8 compassos, FEEL (swing). Toca no
   relógio do sequenciador. Salvo no projeto (track de 56 bytes, versão 9) e nos sons. Kits de fábrica trazem o estilo.
4. **Melódico:** baixo tumbao e piano montuno que seguem o acorde; metais em FM; cuatro dedilhado.
5. **Mood:** GENERATE vira NOCHE / CALLE / FIESTA (variações sempre na clave). FX: reverb de rua ou quintal,
   delay de dub, "rádio AM" (lo-fi). Telas neon.
6. **Publicação:** testes host e web, teste no FM-1 físico (instalar, sair para outro firmware, retomar uma
   atualização interrompida), release com o workflow `zvenson/fm1-build`, `fmhub.json` (`kinds: ["groove"]`),
   card 240×240.

## Orçamento (GR!MM 2.0.2 sem mudanças, medido em 2026-11-05)

| Recurso | Usado | Limite | Livre |
| --- | --- | --- | --- |
| Código + constantes | 506 KB | 581 KB | ~75 KB |
| RAM `.data` + `.bss` | 88 KB | 96 KB | ~10 KB |
| Pool | 342 KB | 344 KB | ~2 KB |
| Pilha do usuário (pior caso) | 3 KB | 24 KB | 21 KB |

A pool está praticamente cheia (buffers de reverb, delay, grain e tape). Para caber a percussão e os FX
latinos, vamos tirar fontes que não combinam com o mood: os coros do VOICE, o PHYS e talvez o GRAIN. A
percussão sintetizada quase não usa pool; o espaço vai para os FX novos.

## Ambiente

- Toolchain: `~/.jieli/toolchain` (jieli-linux-toolchains-20260730.1). SDK: `~/fw-AC79_AIoT_SDK`.
- Python: `.venv` no projeto (criado com `uv`). Compilar com `PATH="$PWD/.venv/bin:$PATH" ./build.sh`.
- `python-rtmidi` não carrega no WSL (falta libasound). Instalar no FM-1 pelo Chrome/Edge no Windows.

## Regras do chip (de `docs/RULES.md`)

- Sem `float`/`double`, sem divisão de 64 bits. Ponto fixo Q15.
- `app_audio()`: 256 frames a cada 5,8 ms. Meta: menos de 2,9 ms.
- Nada de flash, tela ou espera dentro do áudio. Salvar só em silêncio.
