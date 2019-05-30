pragma solidity ^0.5;

contract Addition {

    function add(int left) public pure returns(int result){
        if(left==3)
            return 10;
        else{
            return 5;
        }
    }
}
