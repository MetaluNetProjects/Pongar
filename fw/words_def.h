
// 40 "nombres" (un examplaire chaque):
WORDS_MACRO(_0 ,        0)     // zéro
WORDS_MACRO(_1 ,        1)     // un
WORDS_MACRO(_2 ,        2)
WORDS_MACRO(_3 ,        3)
WORDS_MACRO(_4 ,        4)
WORDS_MACRO(_5 ,        5)
WORDS_MACRO(_6 ,        6)
WORDS_MACRO(_7 ,        7)
WORDS_MACRO(_8 ,        8)
WORDS_MACRO(_9 ,        9)
WORDS_MACRO(_10,        10)
WORDS_MACRO(_11,        11)
WORDS_MACRO(_12,        12)
WORDS_MACRO(_13,        13)
WORDS_MACRO(_14,        14)
WORDS_MACRO(_15,        15)
WORDS_MACRO(_16,        16)
WORDS_MACRO(_17,        17)
WORDS_MACRO(_18,        18)
WORDS_MACRO(_19,        19)
WORDS_MACRO(_20,        20)    // "vinte"

WORDS_MACRO(_30,        21)
WORDS_MACRO(_40,        22)
WORDS_MACRO(_50,        23)
WORDS_MACRO(_60,        24)
WORDS_MACRO(_70,        25)
WORDS_MACRO(_80,        26)
WORDS_MACRO(_90,        27)
WORDS_MACRO(_100,       28)
WORDS_MACRO(_1000,      29)
WORDS_MACRO(et,         30)    // et
WORDS_MACRO(une,        31)    // une

WORDS_MACRO(virgule,    32)
WORDS_MACRO(centieme,   33)
WORDS_MACRO(millieme,   34)
/*
WORDS_MACRO(et,         21)    // et
WORDS_MACRO(une,        22)    // une
WORDS_MACRO(_30,        23)
WORDS_MACRO(_40,        24)
WORDS_MACRO(_50,        25)
WORDS_MACRO(_60,        26)
WORDS_MACRO(_70,        27)
WORDS_MACRO(_80,        28)
WORDS_MACRO(_90,        29)
WORDS_MACRO(_100,       30)
WORDS_MACRO(_1000,      31)
*/

#define WORDS_MULTIPLE_NITEMS 5
#define WORDS_MULTIPLE_OFFSET 40
#define WORDS_MULTIPLE(n) (WORDS_MULTIPLE_OFFSET + WORDS_MULTIPLE_NITEMS * n)
WORDS_MACRO(partie                  , WORDS_MULTIPLE(0))
WORDS_MACRO(joueur                  , WORDS_MULTIPLE(1))
WORDS_MACRO(niveau                  , WORDS_MULTIPLE(2))

WORDS_MACRO(minute                  , WORDS_MULTIPLE(3))
WORDS_MACRO(seconde                 , WORDS_MULTIPLE(4))
WORDS_MACRO(place                   , WORDS_MULTIPLE(5))

WORDS_MACRO(alpague                 , WORDS_MULTIPLE(7))    // Allez, venez jouer au Pong !
WORDS_MACRO(alpague2                , WORDS_MULTIPLE(8))    
WORDS_MACRO(alpague3                , WORDS_MULTIPLE(9))    
WORDS_MACRO(hiscore_jour            , WORDS_MULTIPLE(10))   // Nous vous rappelons que le meilleur temps aujourd'hui est de:
WORDS_MACRO(hiscore_general         , WORDS_MULTIPLE(11))   // Nous vous rappelons que le meilleur temps au classement général est de:

WORDS_MACRO(bonjour                 , WORDS_MULTIPLE(12))   // Bienvenue, placez-vous autour de la piste. Combien de joueurs êtes-vous ?
WORDS_MACRO(attente_joueurs_stables , WORDS_MULTIPLE(13))   // S'il vous plait, arretez de vous déplacer pour que je puisse savoir combien vous êtes.

WORDS_MACRO(trop_nombreux           , WORDS_MULTIPLE(14))   // Attention vous êtes trop nombreux, 4 joueurs max.
WORDS_MACRO(debut_partie            , WORDS_MULTIPLE(15))   // Très bien la partie va pouvoir commencer. Vous êtes : ("un" "joueur")
WORDS_MACRO(gagne_niveau_1          , WORDS_MULTIPLE(16))   // Bien bien, vous avez reporté cette première manche ! Préparez-vous pour la prochaine, ça va être un peu plus difficile. Vous avez trois vies.
WORDS_MACRO(gagne_niveau_2          , WORDS_MULTIPLE(17))   // Bravo, vous avez reporté la deuxième manche ! Attention, la troisième est vraiment coton.
WORDS_MACRO(perdu_niveau            , WORDS_MULTIPLE(18))   // Dommage, vous avez perdu. Mais vous pouvez réessayer : il vous reste deux vies.
WORDS_MACRO(perdu_niveau_derniere   , WORDS_MULTIPLE(19))   // Encore perdu ! Attention c'est votre dernière chance ! Vous n'avez plus qu'une vie !
WORDS_MACRO(gagne_partie            , WORDS_MULTIPLE(20))   // Bravo, vous avez remporté la dernière manche ! Votre temps est de : (Pour remporter ces trois manches, vous avez mis :).
WORDS_MACRO(classement_jour         , WORDS_MULTIPLE(21))   // Votre place dans le classement du jour est :
WORDS_MACRO(champion_jour           , WORDS_MULTIPLE(22))   // 1: bravo meilleur score de la journée ! 2: incroyable, deuxième place du jour ! etc.
WORDS_MACRO(classement_general      , WORDS_MULTIPLE(23))   // Vous êtes rentré dans le classement général ; vous avez conquis la place numéro :
WORDS_MACRO(champion                , WORDS_MULTIPLE(24))   // 1: bravo champion du monde ! 2: incroyable, deuxième place ! etc. jusque 5
WORDS_MACRO(perdu_niveau1           , WORDS_MULTIPLE(25))   // Vous avez perdu dès le premier niveau, laissez la place à des meilleurs que vous.
WORDS_MACRO(perdu_partie            , WORDS_MULTIPLE(26))   // Zut, c'était votre dernière chance, et vous avez perdu la partie ! N'hésitez pas à réessayer une prochaine fois.

WORDS_MACRO(perdu                   , WORDS_MULTIPLE(29))
WORDS_MACRO(gagne                   , WORDS_MULTIPLE(30))
WORDS_MACRO(vies                    , WORDS_MULTIPLE(32))
WORDS_MACRO(derniere_vie            , WORDS_MULTIPLE(33)) // attention dernière vie !

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

/*
WORDS_MACRO(partie                  , 101)
WORDS_MACRO(joueur                  , 102)
WORDS_MACRO(perdu                   , 103)
WORDS_MACRO(gagne                   , 104)
WORDS_MACRO(niveau                  , 105)
WORDS_MACRO(vies                    , 106)
WORDS_MACRO(derniere_vie            , 107) // attention dernière vie !
*/

