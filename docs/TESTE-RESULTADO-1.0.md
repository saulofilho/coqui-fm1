# Resultado do teste no FM-1 (1.0)

Teste feito em 2026-11-06 num M-VAVE FM-1 de verdade, ligado por USB a um notebook com Windows, seguindo
[TESTE-FM1.md](TESTE-FM1.md), com o pacote `coqui.fwsc` do commit da 1.0 (identidade `FM-1_923`). Ferramentas:
`tools/fm1_install.py` e `tools/fm1_cpu.py` (Python 3.12, mido, python-rtmidi).

## Parte A: o que o FM-1 Firmware Hub exige

| # | Passo | Resultado |
|---|---|---|
| 1 | Partir do oficial | ✅ o FM-1 respondia `FM-1_015` (V15 da M-VAVE) |
| 2 | Instalar a partir do V15 | ✅ instalou, reiniciou no COQUÍ |
| 3 | Identidade pela USB | ✅ `FM-1_923` (porta "Felucca 0") |
| 4 | Sair do COQUÍ | ✅ o GR!MM (`FM-1_966`) instalado por cima pelo Firmware Hub, ligou |
| 5 | Instalar a partir de outro firmware | ✅ do GR!MM (`FM-1_966`) para o COQUÍ: "done: the FM-1 runs FM-1_923" |
| 6 | Atualização interrompida | ✅ cabo tirado no meio da gravação; o instalador achou o loader (`ota-FM-1_900`, "in update mode: the write will be finished"), terminou, `FM-1_923` |
| 7 | Rescue (OCT− ao ligar) | ✅ (relatado pelo testador: tudo correto) |
| 8 | Voltar ao oficial | não feito (opcional; o caminho de saída já está provado no passo 4) |

## Parte B: o COQUÍ

Tocado de ouvido: o boot, os grooves, as linhas (TUMBAO BASS, MONTUNO PIANO), os sons de fábrica, o GENERATE e o
AMBIENTE "parecem tudo ok". Nenhum defeito relatado.

## Parte C: o processador

A cena mais pesada (T1 SALSA DURA com GROOVE, T2 TUMBAO BASS e T3 MONTUNO PIANO com LATCH, T4 MAMBO BRASS com GENERATE
em FIESTA, AMBIENTE DUB, PLAY), `fm1_cpu.py --once` depois de ~20 s:

```
NOW  31.8%  PEAK  47.3%  |  VOICES 16.9  MIX  3.5  GRAIN  0.4  DELAY  0.2  SPACE  7.9  TAPE  0.0  PERFORM  0.6  MASTER  1.1  DRIVE  0.0  SCAN  1.1  REST  0.0  |  USB REC DROPS 0
```

Folga de mais da metade do bloco de 5,8 ms. Saúde (`--health`, com o FM-1 parado): watchdog com 617,7 ms do limite de
~8000, varredura de teclas sem atrasos, flash sem interrupções longas, USB sem perdas, 0 resets inesperados, 0 falhas.

## Observação

O instalador do Windows precisa do Python 3.12: o `python-rtmidi` não tem pacote pronto para versões mais novas (o
roteiro já diz isso).
