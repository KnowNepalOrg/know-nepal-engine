// Standalone extraction script — runs from know-nepal-engine/benchmark/
// Reads the static-index.ts file and evaluates it to extract the search index.
// No external imports needed.

import * as fs from "fs";
import * as path from "path";
import * as vm from "vm";

const STATIC_INDEX_PATH = path.resolve(
  __dirname,
  "../../know-nepal/know-nepal-platform/know-nepal-frontend/src/shared/search/static-index.ts"
);

// Read the TypeScript source
const source = fs.readFileSync(STATIC_INDEX_PATH, "utf-8");

// We need to evaluate the module in a sandbox. The static-index.ts file:
// 1. Imports ModuleKey from @/shared/lib/constants (we'll mock this)
// 2. Defines seed data arrays
// 3. Pushes entries into an `entries` array
// 4. Exports STATIC_SEARCH_INDEX = entries

// Strategy: strip the import line and mock the type, then evaluate
const modifiedSource = source
  .replace(/import\s+.*from\s+["'].*["'];?\n/g, "")
  .replace(/export\s+type\s+.*\n/g, "")
  .replace(/export\s+/g, "");

// Create a sandbox with minimal mocks
const sandbox = {
  console,
  module: { exports: {} },
  exports: {},
  Array,
  Object,
  Math,
  String,
  Number,
  Boolean,
  RegExp,
  Map,
  Set,
  Date,
  JSON,
  parseInt,
  parseFloat,
  isNaN,
  isFinite,
  undefined,
  NaN,
  Infinity,
  globalThis: {},
};

try {
  const script = new vm.Script(modifiedSource, { filename: "static-index.js" });
  const context = vm.createContext(sandbox);
  script.runInContext(context);

  // The entries should be in sandbox.module.exports or sandbox.exports
  // Let's look for STATIC_SEARCH_INDEX or entries
  const exports = sandbox.module.exports;
  const index = exports.STATIC_SEARCH_INDEX || exports.default || exports.entries;

  if (!index || !Array.isArray(index)) {
    // Try to find it in the global sandbox
    console.error("Could not find STATIC_SEARCH_INDEX in exports.");
    console.error("Exports keys:", Object.keys(exports));
    process.exit(1);
  }

  const outPath = path.join(__dirname, "dataset.json");
  fs.writeFileSync(outPath, JSON.stringify(index, null, 2));
  console.log(`Extracted ${index.length} entries -> ${outPath}`);
} catch (e) {
  console.error("Failed to evaluate static-index.ts:", e);
  process.exit(1);
}
