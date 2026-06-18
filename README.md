# mpi-deadlock-gcc-plugin

MPI plugin pour gcc v12.2.0

Compiler plugin:

```
make
```

Compiler les test (tester le plugin):

```
make <nom_du_fichier_test>
make test-fail-1 # devrait donner des warnings 
make test-pass-1 # ne devrait pas donner des warnings 
```

Option de debug pour générer un graphviz des fonctions:

```
make debug
```

Générer les png des .dot et les mettre dans png/:

```
make dot2png
```

Clean :

```
make clean 
```

Clean les .dot et les png:

```
make cleanall
```
