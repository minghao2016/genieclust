"""
Genieclust Python Package
Copyright (C) 2018 Marek.Gagolewski.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
"""

from setuptools import setup
from distutils.extension import Extension
from distutils.command.sdist import sdist as _sdist
from Cython.Distutils import build_ext


class sdist(_sdist):
    def run(self):
        # make sure the distributed .pyx files are up-to-date
        from Cython.Build import cythonize
        cythonize(["genieclust/argkmin.pyx"])
        cythonize(["genieclust/compare_partitions.pyx"])
        cythonize(["genieclust/disjoint_sets.pyx"])
        cythonize(["genieclust/genie.pyx"])
        cythonize(["genieclust/hdbscan.pyx"])
        cythonize(["genieclust/inequity.pyx"])
        cythonize(["genieclust/mst.pyx"])
        _sdist.run(self)

cmdclass = {}
cmdclass["sdist"] = sdist

ext_modules = [ ]


ext_modules += [
    Extension("genieclust.inequity", [ "genieclust/inequity.pyx" ]),
    Extension("genieclust.compare_partitions", [ "genieclust/compare_partitions.pyx" ]),
    Extension("genieclust.disjoint_sets", [ "genieclust/disjoint_sets.pyx" ]),
    Extension("genieclust.mst", [ "genieclust/mst.pyx" ]),
    Extension("genieclust.genie", [ "genieclust/genie.pyx" ]),
    Extension("genieclust.hdbscan", [ "genieclust/hdbscan.pyx" ]),
    Extension("genieclust.argkmin", [ "genieclust/argkmin.pyx" ]),
]
cmdclass.update({ 'build_ext': build_ext })



setup(
    name="genieclust",
    version="1.0",
    description="The Genie Clustering Algorithm Suite",
    author="Marek Gagolewski",
    author_email="marek@gagolewski.com",
    maintainer="Marek Gagolewski",
    maintainer_email="marek@gagolewski.com",
    license="GNU GPL v3 or later",
    install_requires=[
          "numpy",
          "scipy",
          "cython",
          "matplotlib",
          "sklearn"
      ],
    #download_url="https://www.github.com/gagolews/genieclust",
    url="http://www.gagolewski.com/",
    cmdclass=cmdclass,
    ext_modules=ext_modules
)