pragma solidity ^0.5;

contract Addition {

    function add(int left) public pure returns(string memory result){
        string memory statictext = "HelloStackOverFlow";
        if(left==3)
            return "Hello";
        else{
            return statictext;
        }
    }
} 
