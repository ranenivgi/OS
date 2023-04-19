#!/bin/bash
#Ranen Ivgi 208210708

ball_state=0
player1_points=50
player2_points=50
game_over=0

print_board () {
    echo " Player 1: ${player1_points}         Player 2: ${player2_points} "
    echo -e " --------------------------------- \n |       |       #       |       | \n |       |       #       |       | "

    case $ball_state in
    3)
        echo " |       |       #       |       |O"
        ;;
    2)
        echo " |       |       #       |   O   | "
        ;;
    1)
        echo " |       |       #   O   |       | "
        ;;
    0)
        echo " |       |       O       |       | "
        ;;
    -1)
        echo " |       |   O   #       |       | "
        ;;
    -2)
        echo " |   O   |       #       |       | "
        ;;
    -3)
        echo "O|       |       #       |       | "
        ;;
    esac
    echo -e " |       |       #       |       | \n |       |       #       |       | \n --------------------------------- "

}

read_picks() {
    if [[ $1 == 1 ]]; then
        echo "PLAYER 1 PICK A NUMBER: " >&2
    elif [[ $1 == 2 ]]; then
        echo "PLAYER 2 PICK A NUMBER: " >&2
    fi
    read -s number

    #check for number validation
    while (! [[ "$number" =~ ^[0-9]+$ ]]) || [ "$number" -lt "0" ] || [ "$number" -gt "$2" ]; do
        echo "NOT A VALID MOVE !" >&2
        if [[ $1 == 1 ]]; then
            echo "PLAYER 1 PICK A NUMBER: " >&2
        elif [[ $1 == 2 ]]; then
            echo "PLAYER 2 PICK A NUMBER: " >&2
        fi
        read -s number
    done

    #return the number
    echo $number
 }

change_ball_state() {
    if [ "$1" -gt "$2" ]; then
        if [ "$ball_state" -lt "1" ]; then
            ball_state=1
        else 
            ball_state=$(($ball_state + 1))
        fi
    elif [ "$1" -lt "$2" ]; then
        if [ "$ball_state" -gt "-1" ]; then
            ball_state=-1
        else
            ball_state=$(($ball_state - 1))
        fi
    fi    
}

game() {
    #get moves from the players
    player1_choice=$(read_picks "1" "$player1_points" | tail -n "1")
    player2_choice=$(read_picks "2" "$player2_points" | tail -n "1")

    #change the ball state after the move
    change_ball_state "$player1_choice" "$player2_choice"

    #remove points from the players
    player1_points=$(($player1_points - $player1_choice))
    player2_points=$(($player2_points - $player2_choice))

    #print the board and the players' choices
    print_board
    echo -e "       Player 1 played: ${player1_choice}\n       Player 2 played: ${player2_choice}\n\n"
}

check_win() {

    if [ $ball_state == "3" ] || ( [ $player1_points -gt "0" ] && [ $player2_points == "0" ] ) || ( [ $player1_points == "0" ] && [ $player2_points == "0" ] && [ $ball_state -gt "0" ] ); then
        echo "PLAYER 1 WINS !"
        game_over=1
    fi

    if [[ $ball_state == "-3" ]] || ( [ $player1_points == "0" ] && [ $player2_points -gt "0" ] ) || ( [ $player1_points == "0" ] && [ $player2_points == "0" ] && [ $ball_state -lt "0" ] ); then
        echo "PLAYER 2 WINS !"
        game_over=1
    fi
}

main() {
    print_board
    while [ $game_over == "0" ]
    do
        game
        check_win
    done
}

main
