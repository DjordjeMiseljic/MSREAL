#Prvi drajver
Cilj ove vezbe je da se demonstriraju napredni kernel objekti tako sto ce se lifo drajver sa proslog casa unaprediti.
Potrebno je napraviti blokiranje procesa koji pokusavaju da citaju iz prazanog lifo registra, kao i onih koji pokusavaju da upisu u pun lifo registar. Takodje je potrebno implementirati pomocu semafora da jedan proces moze u jednom trenutku pristupiti deljenom resursu (lifo registru). Na ovaj nacin se nece izgubiti ni jedan podatak koji se pokusa upisati u lifo. Ovo je demonstrirano u direktorijumu Lifo_Blokiranje+Semafori
U direktorijumu Lifo_Asinhrono+Aplikacija je demonstriran primer asinhronog obavestavanja aplikacije od strane drajvera pomocu signala.
