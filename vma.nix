{ lib
, stdenv
, fetchgit
, cmake
}:

stdenv.mkDerivation (finalAttrs: rec {
  pname = "VulkanMemoryAllocator";
  version = "0.17";

  src = fetchgit {
    url = "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git";
    hash = "sha256-6Dj5icK3Gx7mcnICChhE30zB2N0VNEzvV1D3Q37ZIGY=";
  };

  outputs = [ "out" "dev" ];

  nativeBuildInputs = [ cmake ];
  
  configurePhase = ''
    mkdir -p $out
    cmake -S $src -B $out
  '';

  installPhase = ''
    cmake --install $out --prefix $out/install
  '';
  
  meta = with lib; {
    description = "VulkanMemoryAllocator";
    homepage = "https://www.libsdl.org/";
    platforms = platforms.unix;
    license = licenses.mit;
  };
})