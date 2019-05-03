pragma solidity ^0.5;

contract StringConcat {


    function add(string memory s1, string memory s2) public pure returns(string memory result){
    
        bytes memory b1 = bytes(s1);
        bytes memory b2 = bytes (s2);
        
        string memory concat = new string(b1.length + b2.length);
        bytes memory bconcat = bytes(concat);
        
        uint k = 0;
        for(uint i = 0; i<b1.length;i++){
            bconcat[k++] = b1[i];
        }
        
        //second for loop missing
        
        return string(bconcat);
    }
} 
