
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

WORDS_MACRO(minute,     35)
WORDS_MACRO(seconde,    36)
WORDS_MACRO(joueur,     37)
WORDS_MACRO(parmi,      38)

#define WORDS_MULTIPLE_NITEMS 5
#define WORDS_MULTIPLE_OFFSET 40
#define WORDS_MULTIPLE(n) (WORDS_MULTIPLE_OFFSET + WORDS_MULTIPLE_NITEMS * n)
// 43 WORDS_MULTIPLE maximum (40 + 43 * 5) = 255

WORDS_MACRO(vous_etes_trop_pres     , WORDS_MULTIPLE(0))    // Attention, il est interdit de vous approcher trop près du Pong, ne franchissez pas la limite
WORDS_MACRO(trop_pres               , WORDS_MULTIPLE(1))    // "Trop près !" "Vous avez mordu la ligne" (doit être court)
WORDS_MACRO(temps_ecoule            , WORDS_MULTIPLE(2))    // Vous avez dépassé le temps maximum imparti. Vous avez perdu la partie.
WORDS_MACRO(pas_extra_balle         , WORDS_MULTIPLE(3))    // Vous avez fait trop de fautes lors de cette première manche, vous n'avez droit à aucune extra balle.
WORDS_MACRO(une_extra_balle         , WORDS_MULTIPLE(4))    // Votre performance moyenne lors de cette manche de qualification vous donne droit à une extra balle.
WORDS_MACRO(deux_extra_balles       , WORDS_MULTIPLE(5))    // Bravo, votre bonne performance lors de cette manche de qualification vous donne droit à deux extra balles.

WORDS_MACRO(alpague                 , WORDS_MULTIPLE(7))    // Allez, venez jouer au Pong !
WORDS_MACRO(alpague2                , WORDS_MULTIPLE(8))    // donner les règles du jeu
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

WORDS_MACRO(temps_intermediaire     , WORDS_MULTIPLE(27))    // Votre temps de parcours est pour l'instant de:
WORDS_MACRO(nombre_vainqueurs       , WORDS_MULTIPLE(28))    // Le nombre total de joueurs ayant gagné la partie est de:

#undef WORDS_MACRO // need to redefine WORDS_MACRO each time
