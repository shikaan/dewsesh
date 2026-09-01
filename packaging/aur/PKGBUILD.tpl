# Maintainer: Manuel Spagnolo <shikaan@disroot.org>

# Template. The 'pkgbuild' job in .github/workflows/release.yml renders this
# into the PKGBUILD attached to each release. This file is not a usable
# PKGBUILD.
pkgver=##VERSION##
sha256sums=('##CHECKSUM##')
_commit=##SHA##

pkgname=dewsesh
pkgrel=1
pkgdesc="A minimal, beautiful session manager for Wayland"
arch=('x86_64' 'aarch64')
url="https://github.com/shikaan/dewsesh"
license=('MIT')
depends=('wayland' 'cairo' 'freetype2')
# wayland-scanner ships in 'wayland', which is already a runtime dependency.
makedepends=('scdoc')
source=("$pkgname-$pkgver.tar.gz::$url/archive/refs/tags/v$pkgver.tar.gz")

build() {
  cd "$pkgname-$pkgver"
  make VERSION="v$pkgver" SHA="$_commit" ERR_ON_WARN=0 all
}

package() {
  cd "$pkgname-$pkgver"
  make install VERSION="v$pkgver" SHA="$_commit" DESTDIR="$pkgdir" PREFIX=/usr
  install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
