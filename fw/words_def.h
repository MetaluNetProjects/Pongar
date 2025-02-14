
// 40 "nombres" (un examplaire chaque):
WORDS_MACRO(_0 , 0)     // zéro
WORDS_MACRO(_1 , 1)
WORDS_MACRO(_2 , 2)
WORDS_MACRO(_3 , 3)
WORDS_MACRO(_4 , 4)
WORDS_MACRO(_5 , 5)
WORDS_MACRO(_6 , 6)
WORDS_MACRO(_7 , 7)
WORDS_MACRO(_8 , 8)
WORDS_MACRO(_9 , 9)
WORDS_MACRO(_10, 10)
WORDS_MACRO(_11, 11)
WORDS_MACRO(_12, 12)
WORDS_MACRO(_13, 13)
WORDS_MACRO(_14, 14)
WORDS_MACRO(_15, 15)
WORDS_MACRO(_16, 16)
WORDS_MACRO(_17, 17)
WORDS_MACRO(_18, 18)
WORDS_MACRO(_19, 19)
WORDS_MACRO(_20, 20)    // "vinte"
WORDS_MACRO(_21, 21)    // Vingt-et-une
WORDS_MACRO(_30, 22)
WORDS_MACRO(_31, 23)    // Tente-et-une
WORDS_MACRO(_40, 24)
WORDS_MACRO(_41, 25)    // Quarante-et-une
WORDS_MACRO(_50, 26)
WORDS_MACRO(_51, 27)    // Cinquante-et-une
WORDS_MACRO(_60, 28)
WORDS_MACRO(_61, 29)    // Soixante-et-une
WORDS_MACRO(_70, 30)
WORDS_MACRO(_71, 31)    // Soixante-et-onze
WORDS_MACRO(_80, 32)
WORDS_MACRO(_81, 33)    // Quatre-vingt-une
WORDS_MACRO(_90, 34)
WORDS_MACRO(_91, 35)    // Quatre-vingt-onze (?)
WORDS_MACRO(_100, 36)
WORDS_MACRO(_1000, 37)
WORDS_MACRO(virgule, 38)

WORDS_MACRO(partie                  , 101)
WORDS_MACRO(joueur                  , 102)
WORDS_MACRO(perdu                   , 103)
WORDS_MACRO(gagne                   , 104)
WORDS_MACRO(niveau                  , 105)
WORDS_MACRO(vies                    , 106)
WORDS_MACRO(derniere_vie            , 107) // attention dernière vie !

#undef WORDS_MACRO // need to redefine WORDS_MACRO each time

/* TODO:

// 43 "phrases" max, 5 choix ( 43 * 5 + 40 = 255)

partie
joueur
minute
seconde
place

alpague                 // Allez, venez jouer au Pong !
alpague2
alpague3
hiscore_jour            // Nous vous rappelons que le meilleur temps aujourd'hui est de:
hiscore_general         // Nous vous rappelons que le meilleur temps au classement général est de:

bonjour                 // Bienvenue, placez-vous autour de la piste. Combien de joueurs êtes-vous ?
attente_joueurs_stables // S'il vous plait, arretez de vous déplacer pour que je puisse savoir combien vous êtes.
trop_nombreux           // Attention vous êtes trop nombreux, 4 joueurs max.
début_partie            // Très bien la partie va pouvoir commencer. Vous êtes : ("un" "joueur")
gagne_niveau_1          // Bien bien, vous avez reporté cette première manche ! Préparez-vous pour la prochaine, ça va être un peu plus difficile. Vous avez trois vies.
gagne_niveau_2          // Bravo, vous avez reporté la deuxième manche ! Attention, la troisième est vraiment coton.
perdu_niveau            // Dommage, vous avez perdu. Mais vous pouvez réessayer : il vous reste deux vies.
perdu_niveau_derniere   // Encore perdu ! Attention c'est votre dernière chance ! Vous n'avez plus qu'une vie !
gagne_partie            // Bravo, vous avez remporté la dernière manche ! Votre temps est de : (Pour remporter ces trois manches, vous avez mis :).
classement_jour         // Votre place dans le classement du jour est :
champion_jour           // 1: bravo meilleur score de la journée ! 2: incroyable, deuxième place du jour ! etc.
classement_general      // Vous êtes rentré dans le classement général ; vous avez conquis la place numéro :
champion                // 1: bravo champion du monde ! 2: incroyable, deuxième place ! etc. jusque 5
perdu_niveau1           // Vous avez perdu dès le premier niveau, laissez la place à des meilleurs que vous.
perdu_partie            // Zut, c'était votre dernière chance, et vous avez perdu la partie ! N'hésitez pas à réessayer une prochaine fois.

*/
