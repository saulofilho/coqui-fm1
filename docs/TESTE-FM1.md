# Roteiro de teste no FM-1 (antes do release 1.0)

O FM-1 Firmware Hub só lista firmwares testados num FM-1 de verdade: instalado a partir do oficial (V15) e a partir de
outro firmware, capaz de sair de novo (outro firmware instala por cima), respondendo pela USB com a identidade `FM-1_`
própria, e retomando uma atualização interrompida. Este roteiro cobre isso, e depois o próprio COQUÍ.

Anote o resultado de cada passo (✅ / ❌ e o que viu) na tabela do fim: é ela que diz se dá para lançar a 1.0.

## Antes de começar

- **Backup.** O COQUÍ não lê os saves de outro firmware, e uma instalação pode apagar o que não for dele. Salve o que
  importa com as ferramentas do firmware que está no FM-1 agora.
- **Windows, Chrome ou Edge**, o FM-1 num cabo USB de dados (não só de carga).
- **O pacote:** `build/coqui.fwsc` (`./build.sh` no WSL) ou o `.fwsc` de um release. Copie para uma pasta do Windows.
- **Python no Windows** (não no WSL: o WSL não enxerga o MIDI USB), uma vez:

  ```
  py -m pip install mido python-rtmidi
  ```

  Os comandos abaixo rodam no PowerShell, dentro de uma cópia do repositório no Windows (ou com o caminho de `tools\`).
- **Saber o modelo do flash:** FM-1s vendidos desde o outono de 2026 têm outro chip (UC25HQ80). O COQUÍ aceita os dois;
  se o seu for novo, o passo 9 é o que mais importa.
- **Se algo der errado:** segure **OCT−** ao ligar (rescue USB do COQUÍ); se nem isso, a página **Rescue** do Firmware
  Hub (thefirmwarehub.com/tools/rescue/) ou o [FM-1-transporter](https://github.com/kurogedelic/FM-1-transporter)
  devolvem o firmware oficial.

## Parte A: o que o Firmware Hub exige

| # | Passo | Como | O que deve acontecer |
|---|---|---|---|
| 1 | Partir do oficial | Instale o V15 oficial da M-VAVE (Hub > Rescue, ou o updater da M-VAVE) | O FM-1 liga no firmware oficial |
| 2 | Instalar a partir do V15 | `py tools\fm1_install.py coqui.fwsc` | Instala sem erro, reinicia, a tela de boot mostra o coquí cantando |
| 3 | Identidade | `py tools\fm1_install.py --info` | Responde `FM-1_923` |
| 4 | Sair do COQUÍ | Instale outro firmware por cima (ex.: GR!MM ou Felucca pelo Hub) | Instala normalmente; o outro firmware liga |
| 5 | Instalar a partir de outro firmware | Com esse outro firmware no FM-1, instale o COQUÍ de novo (passo 2) | Instala; o COQUÍ liga |
| 6 | Atualização interrompida | Comece `py tools\fm1_install.py coqui.fwsc` e **tire o cabo USB no meio** da escrita; recoloque; rode o mesmo comando de novo | O instalador retoma e termina; o COQUÍ liga |
| 7 | Rescue | Desligue, segure **OCT−** e ligue | Entra no modo de rescue USB; uma instalação funciona dali |
| 8 | Voltar ao oficial | Hub > Rescue (ou M-VAVE) com o COQUÍ instalado | O oficial volta (o caminho de saída existe) |

## Parte B: o COQUÍ em si

| # | Passo | Como | O que deve acontecer |
|---|---|---|---|
| 9 | Saves sobrevivem | Gire PRESETS até SALSA DURA, mude algo, espere ~3 s parado, desligue e ligue | Volta como estava (no flash novo também) |
| 10 | PERC | EDIT > SOURCE > PERC; toque as 27 teclas | Cada tecla um tambor (mapa no MANUAL); OCT−/OCT+ não muda o mapa |
| 11 | GROOVE | PRESETS: SALSA DURA, PLAY | O groove toca; EDIT > GROOVE muda STYLE, PARTS, FILL, FEEL |
| 12 | Linhas | Em T2, PRESETS: TUMBAO BASS; segure Dó; PLAY | Tumbao no compasso do groove; troque de tecla: segue o acorde |
| 13 | GENERATE | Segure FX, toque F#3 (T1); FX + D#4 muda NOCHE / CALLE / FIESTA | Notas na grade, mais densas a cada modo |
| 14 | AMBIENTE | FX várias vezes até AMBIENTE; gire SCENE | Cada cena muda o som (DUB: eco longo; RADIO AM: rádio) |
| 15 | HOME | Volte à HOME tocando; pare e espere 5 s | Congas acendem com o som; o coquí canta e depois dorme |
| 16 | MIDI USB | Toque notas de um DAW / teclado USB nos canais 1–4 | Cada canal um track; PERC toca tambores |
| 17 | MIDI TRS | Um teclado no conector TRS (adaptador MIDI) | As notas chegam |
| 18 | Áudio USB | No computador, escolha "FM-1" como entrada e grave | O som do FM-1 chega ao computador |
| 19 | Fone / saída | Fone e saída de linha | Som limpo, sem estalos |
| 20 | Bateria | Use sem USB | O ícone de bateria aparece no topo da HOME |

## Parte C: o processador (o que só o aparelho mede)

O COQUÍ foi medido só no computador. No FM-1 o limite é 100 % de um bloco de 5,8 ms; acima de ~85 % aparecem estalos
ou notas cortadas (o cabeçalho diz CPU FULL: NOTE DROPPED).

1. Monte a cena mais pesada: T1 SALSA DURA (GROOVE, PARTS TODO), T2 TUMBAO BASS, T3 MONTUNO PIANO, T4 MAMBO BRASS com
   GENERATE em FIESTA, AMBIENTE em DUB, e PLAY. Segure um acorde em T3 e T4.
2. Com ela tocando há uns 20 s, rode:

   ```
   py tools\fm1_cpu.py --health
   ```

   (ou veja a última linha de SYSTEM 2: segure HOME e aperte HOME até SYSTEM 2: CPU NOW / PEAK)
3. Anote NOW, PEAK e a linha VOICES (onde a percussão e as vozes entram).

Se PEAK passar de ~85 %, me mande os números: dá para baixar o número de golpes simultâneos do PERC (10) ou a
densidade do FIESTA.

## O que me mandar

| # | ✅/❌ | O que viu |
|---|---|---|
| 1–8 | | |
| 9–20 | | |
| CPU | | NOW __ % · PEAK __ % · VOICES __ % |
| Flash | | novo (UC25HQ80) / antigo / não sei |
| Ouvido | | o que soou estranho (um tambor, um groove, uma linha, um som de fábrica) |

Com a Parte A toda ✅ e a CPU folgada, a 1.0 pode sair: `git tag v1.0 && git push origin v1.0` (o GitHub compila,
assina e publica), e o e-mail para o Hub (`docs/HUB-EMAIL.md`) vai.
