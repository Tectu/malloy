#pragma once

#include "doctest.hpp"

/*
 * These includes get included by doctest.hpp. However, there is a note:
 *
 * "required includes - will go only in one translation unit!"
 *
 * which means that we have to include them again as we generate multiple translation units.
 */
#include <iostream>
