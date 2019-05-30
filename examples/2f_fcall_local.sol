pragma solidity ^0.5;

contract Addition {

    function add(int b) public pure returns(int result){
        int c = b+5;
        c=sub(c);
        return c;
    }

    function sub(int a) public pure returns (int result){
        result = a+10;
    }

}
