# Limbi Publisher

Fișierul de lucru recomandat este `library-source.json`. A fost creat din cele 30 de intrări din `test.json`, cu explicații și 6 expresii suplimentare (ID 1031–1036). `test.json` a fost păstrat. Intrările sunt drafturi; nivelurile sunt orientative. Expresia 1014 a fost armonizată: „shortly” înseamnă „în scurt timp”, nu obligatoriu „imediat”.

## Pornire

Deschide `dist/LimbiPublisher/LimbiPublisher.exe`. Pentru mutarea pe alt calculator folosește întregul ZIP `dist/LimbiPublisher-Windows.zip`, nu doar executabilul.

1. La Sursa JSON alege `D:\proiecte\Limbi\library-source.json` (fișierul principal, nu copia inclusă în pachet).
2. La Repository local alege `D:\proiecte\Limbi` — dosarul care conține `.git`.
3. „Generează JSON + manifest” validează sursa și scrie `library/content-<sha256>.json` și `library/manifest.json`. Poți inspecta aceste fișiere înainte de publicare.
4. „Publică pe GitHub” regenerează pachetul, face stage și commit numai pentru cele două fișiere generate, apoi `git push origin main`. Dacă există alte fișiere deja staged, oprește operația fără să le modifice. Nu face pull automat, reset sau force push.
5. În aplicația Limbi apasă „Actualizează biblioteca”.

Git for Windows trebuie instalat și autentificat cu drept de scriere în `m0ssa99/Limbi`. Programul folosește Git Credential Manager/SSH existent; nu stochează tokenuri. Un refuz de autentificare sau un conflict remote apare în jurnal. Dacă push eșuează, commitul rămâne local; rezolvă problema Git și reîncearcă. Push trimite și commiturile locale anterioare de pe main, ca orice `git push origin main`.

Publicarea acceptă drafturi pentru testare și nu le marchează automat approved. Când verifici lingvistic o intrare, poți completa `status: approved` și `reviewedBy`. Scriptul PowerShell mai vechi cere în continuare numai intrări aprobate; interfața Publisher permite și seturi de test.

## Editare

Editează numai `library-source.json`, în UTF-8. Păstrează ID-urile pentru corecții și atribuie ID-uri noi pentru sensuri noi. Fiecare publicare reprezintă biblioteca completă; nu elimina intrările vechi când adaugi altele. Nu edita fișierele `content-<sha256>.json` direct și nu șterge pachetele vechi.

Repository-ul trebuie să rămână accesibil public pentru actualizarea automată din aplicația curentă. Dacă îl faci privat, Publisher poate face push în continuare prin Git, dar aplicația de învățare nu va putea descărca biblioteca fără o funcție nouă de autentificare.

## Compilare și testare

`powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\Build-Publisher.ps1`

Utilitarul oferă și generare locală fără interacțiune: `LimbiPublisher.exe --generate <sursa.json> <dosar-iesire> -platform offscreen`. Această comandă nu face commit sau push. Test de inițializare: `--smoke-test -platform offscreen`.
