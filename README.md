# AA_SO
Atividade acadêmica de Sistemas Operacionais 2016.1 UFRRJ.
Simples alocador de memória baseado no Malloc da linguagem C. O alocador utiliza a estratégia de Worst-Fit com Best-Fit. Ao procurar pelo Worst-Fit na heap, caso haja um bloco com espaço ideal, a alocação é direta em um Best-Fit local. O programa tem um alto custo por precisar percorrer a heap, porém fragmenta pouco a memória.
