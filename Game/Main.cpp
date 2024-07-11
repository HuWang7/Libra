#include "Engine/Core/StringUtils.hpp"


Strings s1 = SplitStringOnDelimiter("Amy,Bret,Carl", ','); // split into 3 substrings: "Amy", "Bret", "Carl"
Strings s2 = SplitStringOnDelimiter(" -7.5, 3 ", ',');     // split into 2: " -7.5" and " 3 " (including whitespace!)
Strings s3 = SplitStringOnDelimiter("3~7", '~');           // split into 2: "3" and "7"
Strings s4 = SplitStringOnDelimiter("255, 128, 40", ',');  // split into 3: "255", " 128", and " 40" (including spaces!)
Strings s5 = SplitStringOnDelimiter("apple", '/');         // split into 1: "apple"
Strings s6 = SplitStringOnDelimiter("8/2/1973", '/');      // split into 3: "8", "2", and "1973"
Strings s7 = SplitStringOnDelimiter(",,", ',');            // split into 3: "", "", and ""
Strings s8 = SplitStringOnDelimiter(",,Hello,,", ',');     // split into 5: "", "", "Hello", "", and ""
Strings s9 = SplitStringOnDelimiter("", ',');              // split into 1: ""
