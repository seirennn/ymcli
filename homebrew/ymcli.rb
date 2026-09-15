class Ymcli < Formula
  desc "Production-quality terminal YouTube Music client"
  homepage "https://github.com/Seirennn/ymcli"
  url "https://github.com/Seirennn/ymcli/archive/refs/tags/v0.1.0.tar.gz"
  sha256 "0000000000000000000000000000000000000000000000000000000000000000"
  license "MIT"

  depends_on "cmake" => :build
  depends_on "pkg-config" => :build
  depends_on "mpv"
  depends_on "yt-dlp"
  depends_on "openssl@3"
  depends_on "sqlite"

  def install
    system "cmake", "-S", ".", "-B", "build", *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    assert_match "ymcli", shell_output("#{bin}/ymcli --help 2>&1", 1)
  end
end
