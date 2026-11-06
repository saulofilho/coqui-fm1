# E-mail para o FM-1 Firmware Hub (rascunho)

Mandar só depois do roteiro `docs/TESTE-FM1.md` (Parte A toda ✅) e do release 1.0 publicado no GitHub. O Hub pede, por
e-mail: o nome do firmware, o nome do autor como deve aparecer, o link do repositório, a identidade `FM-1_`, a licença
e a declaração de que foi testado num FM-1 de verdade. Texto, imagem e links vão no `fmhub.json` (já está no repo), não
no e-mail; links em vez de anexos.

**Para:** zven@thefirmwarehub.com
**Assunto:** New firmware for the hub: COQUÍ (FM-1_923)

---

Hi zven,

I'd like to submit COQUÍ for the FM-1 Firmware Hub.

- Firmware: COQUÍ, a Latin and Caribbean groovebox for the FM-1 (built on GR!MM 2.0.2)
- Author, as it should appear: saulofilho
- Repository: https://github.com/saulofilho/coqui-fm1
- Identity: FM-1_923 (every release)
- Licence: GPL-3.0-only (LICENSE and LICENSING.md in the repository root; credits to GR!MM, Dinghy, Felucca and SLOOP)
- Releases: GitHub releases with the .fwsc attached, built and attested with zvenson/fm1-build@v1
- Update loader: Felucca's, in the package
- Newer FM-1s: both flash IDs accepted (0x856014 and 0xB36014)
- fmhub.json: in the repository root (card: docs/card.png; play: the web emulator)

Tested on a real FM-1 (2026-11-06): installed from the official V15 (FM-1_015) and from another firmware (GR!MM,
FM-1_966), left again by installing GR!MM over it from the hub, answering FM-1_923 over USB, an interrupted update
resumed by the installer (its loader in update mode, the write finished), the USB rescue (OCT- at power-on) working. The
heaviest scene I could build (four tracks, the groove, two ARP lines, GENERATE, the dub delay) peaks at 47 % of the audio
block. The results: https://github.com/saulofilho/coqui-fm1/blob/main/docs/TEST-RESULTS-1.0.md

Thanks for the hub!

saulofilho
