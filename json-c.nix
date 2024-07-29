{ lib
, stdenv
, fetchgit
, cmake
}:

stdenv.mkDerivation (finalAttrs: rec {
  pname = "json-c";
  version = "0.17";

  src = fetchgit {
    url = "https://github.com/json-c/json-c.git";
    hash = "sha256-iB7NsMlB0eiJtP0RxeJBN0lhAaPqEQQLs/9TYhb5HZY=";
  };

  outputs = [ "out" "dev" ];

  nativeBuildInputs = [ cmake ];
  
  configurePhase = ''
    mkdir -p $out
    cd $out
    cmake $src -DCMAKE_INSTALL_PREFIX="$out"
  '';
  
  meta = with lib; {
    description = "json-c";
    homepage = "https://www.libsdl.org/";
    platforms = platforms.unix;
    license = licenses.mit;
  };
})