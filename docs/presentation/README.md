# Apresentação: COQUÍ para o time de engenharia

Fonte do deck de 18 slides (em português) que conta como o COQUÍ foi feito: inspiração, hardware, arquitetura, PERC,
GROOVE, linhas, mood, visual, hub, desafios, testes, o teste no FM-1, o processo e as lições.

- **Deck publicado (privado, só o dono abre):** https://claude.ai/artifact/KPh9DhvQsyT1ciZziXXrEK (PT) · https://claude.ai/artifact/LeErWJ4L7neT5QpQBn6y6i (EN)
- **PDF para baixar:** [coqui-apresentacao-pt.pdf](coqui-apresentacao-pt.pdf) · [coqui-presentation-en.pdf](coqui-presentation-en.pdf) (20 slides cada)
- Tipo: Artifact "Slides" do claude.ai. Exporta PDF e PPTX pelo próprio deck; o modo apresentador mostra as notas.

## Arquivos

| Caminho | O que é |
|---|---|
| `project/deck.json` | Título, ordem dos slides, seções e fontes |
| `project/slides/<id>.html` | Um `<section>` por slide, com estilos inline; o `<aside>` são as notas de fala |
| `assets/<id>.png` | As imagens; nos slides elas aparecem como `/_blob/<id>`, o endereço delas no deck publicado |

As imagens vêm de `docs/screens/` (telas do firmware, em modo host) e de capturas do hub, do emulador e do editor.

## Editar em outra máquina com o Claude Code

1. Clone o repositório e abra o Claude Code nele, logado na mesma conta do claude.ai.
2. Peça algo como: *"edite a apresentação em docs/presentation e republique em
   https://claude.ai/artifact/KPh9DhvQsyT1ciZziXXrEK"*.
3. O Claude lê o deck publicado (que pode ter edições feitas no navegador, mais novas que este diretório), aplica as
   mudanças e republica no mesmo link, com `root` = `docs/presentation`. Os `/_blob/<id>` continuam valendo lá.
4. Depois, peça para baixar de volta a versão publicada para cá e commite, para o repositório não ficar para trás.

Se o deck publicado se perder, dá para criar um novo a partir destes arquivos: um Artifact do tipo Slides, com
`project/` publicado e as imagens de `assets/` subidas de novo (os endereços `/_blob/<id>` mudam e precisam ser
trocados nos slides).
