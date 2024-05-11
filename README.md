proiect so
Programul trebuie sa primeasca in linia de comanda dupa urmatorul model:
    ./p -o dir_snapuri -s dir_fisiere_corupte dir1 dir2 ....
La prima rulare se recomanda a se sterge fisierul run_count.bin, deoarece acesta contorizeaza nuamrul de rulari si poate se doreste a vedea ce se intampla exact la prima rulare a codului. Evident, daca se sterege acest fisier, programul meu C il va scrie din nou resetand numaratoarea de rulari.