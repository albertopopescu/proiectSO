proiect so
Programul trebuie sa primeasca in linia de comanda dupa urmatorul model:
    ./p -o dir_snapuri -s dir_fisiere_corupte dir1 dir2 ....
La prima rulare se recomanda a se sterge fisierul run_count.bin, deoarece acesta contorizeaza numarul de rulari si poate se doreste a vedea ce se intampla exact la prima rulare a codului. De asemenea se recomanda si stergerea fisierelor ce contin "prev_snap.bin" sau "snapshot.txt" din folderul snapuri. Evident, daca se sterege acest fisier, programul meu C il va scrie din nou resetand numaratoarea de rulari.