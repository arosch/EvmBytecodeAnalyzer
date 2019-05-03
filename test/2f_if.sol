pragma solidity ^0.5;

contract Addition {

    int left =3;

    int128 right = 50000000;

    function add() public view returns(int result){

        if(left==3)
            return 10;
        else{
            return 5;
        }
    }

    function sub() public{
    right =6;
    }
}
