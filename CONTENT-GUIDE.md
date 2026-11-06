# Pregătirea bibliotecii

Codul și biblioteca pot sta în repository-uri separate. Aplicația actuală descarcă dintr-o adresă publică HTTPS; repository-urile private nu sunt încă suportate. Nu introduce tokenuri în fișiere sau în URL. Pentru cod privat și distribuție simplă, păstrează un repository public separat doar cu biblioteca.

Adresa implicită pentru repository-ul actual este:

`https://raw.githubusercontent.com/m0ssa99/Limbi/main/library/manifest.json`

Dacă ramura nu este `main`, schimbă adresa din Setări. Acest fișier trebuie publicat înainte ca actualizarea să funcționeze. Nu este necesar GitHub Pages.

## Flux recomandat pentru conținut

1. Pregătește o listă în Excel sau un fișier de lucru: 30–50 de concepte per lot. Pentru primul set: 100 conversație, 100 serviciu, 100 călătorii; aproximativ 70% expresii, 30% cuvinte.
2. Pentru fiecare concept alege un sens concret și un nivel orientativ A2/B1. Nu traduce toate sensurile unui cuvânt într-o singură intrare.
3. Generează un draft cu echivalente naturale în engleză, română și italiană, explicație scurtă în română și un exemplu echivalent în toate limbile. Definițiile și exemplele sunt texte originale, dacă nu ai o sursă cu drept de reutilizare.
4. Verifică sensul, gramatica, registrul, diacriticele și concordanța dintre cele trei exemple. O a doua verificare AI ajută, dar nu înlocuiește controlul lingvistic. Nivelul este orientativ, nu o certificare CEFR.
5. Păstrează `status: draft` până la verificare. Adaugă `reviewedBy` cu numele persoanei care aprobă. Abia apoi folosește `status: approved`.
6. Rulează scriptul de pregătire și încarcă întregul dosar `library` în repository. Publică întotdeauna biblioteca completă, nu doar diferențele. ID-urile existente nu se schimbă și nu se reutilizează; o corecție de formulare păstrează ID-ul, un sens nou primește alt ID.

## Exemplu de intrare (draft)

```json
[
  {
    "id": 13,
    "kind": "expresie",
    "en": "Let me know.",
    "ro": "Anunță-mă.",
    "it": "Fammi sapere.",
    "meaning": "Îi ceri cuiva să îți comunice o informație când o află.",
    "exampleEn": "Let me know when you arrive.",
    "exampleRo": "Anunță-mă când ajungi.",
    "exampleIt": "Fammi sapere quando arrivi.",
    "level": "A2",
    "topic": "conversatie",
    "origin": "original-ai-draft",
    "status": "draft",
    "reviewedBy": ""
  }
]
```

## Prompt pentru generarea unui lot

> Creează 30 de intrări originale pentru învățarea englezei și italienei de către vorbitori de română. Tema: conversație la serviciu. Nivel orientativ: A2–B1. Proporție: 70% expresii, 30% cuvinte. Folosește structura exemplului JSON. Un singur sens per intrare, traduceri naturale și același context în cele trei exemple. Evită traducerile literale ale idiomurilor, arhaismele și termenii foarte rari. Nu copia definiții sau exemple din dicționare. ID-urile pornesc de la [următorul ID liber]. Nu repeta conceptele din [lista existentă]. Marchează totul draft, cu reviewedBy gol. Returnează doar lista JSON.

Pentru o sursă externă, păstrează și `sourceUrl`, `sourceDictionary`, `sourceDefinitionId`, `license`, `attribution`. Accesul automat la dexonline și redistribuirea trebuie făcute numai pe baza unui set oficial permis sau a acordului corespunzător; nu presupune că toate dicționarele au aceleași condiții. O traducere generată nu elimină condițiile sursei.

## Generarea pachetului

Salvează intrările verificate într-un fișier JSON UTF-8 și rulează:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Prepare-Library.ps1 -Source .\approved.json
```

Scriptul refuză intrările neaprobate, câmpurile goale și ID-urile duplicate. Produce un fișier cu nume bazat pe SHA-256 și `library/manifest.json`. Numele unic evită situațiile în care manifestul și conținutul provin din versiuni diferite ale cache-ului GitHub. Păstrează și pachetele mai vechi. Nu edita manual fișierele generate; regenerează din sursă.

Aplicația verifică la pornire și o dată la 24 de ore în timpul funcționării, plus manual din buton. La erori păstrează biblioteca locală; descărcarea nu modifică progresul. Pachetele pot avea maximum 8 MiB, folosesc HTTPS pe aceeași gazdă ca manifestul și sunt validate înainte de salvarea atomică. SHA-256 verifică potrivirea pachetului cu manifestul, nu este o semnătură digitală a autorului.

Biblioteca inclusă de 12 intrări este demonstrativă și nu a fost declarată automat aprobată. Publică primul set după verificare. În această etapă aplicația nu generează conținut și nu citește direct dexonline.
