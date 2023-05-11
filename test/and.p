/* Truth table for AND, also test the parsing of block comments */
%unsat

fof(a,conjecture,

  ~($false&$false)
& ~($false&$true)
& ~($true&$false)
& ($true&$true)

).
