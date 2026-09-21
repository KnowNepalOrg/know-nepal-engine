// Parity comparison: loads ts_results.json and cpp_results.json, compares them.
// Reports differences in names, scores, matchKind, and ordering.

import * as fs from "fs";
import * as path from "path";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

interface ResultEntry {
  name: string;
  score: number;
  matchKind: string;
  module: string;
  type: string;
}

interface ComparisonReport {
  query: string;
  tsCount: number;
  cppCount: number;
  namesMatch: boolean;
  scoreDiffs: { name: string; tsScore: number; cppScore: number; diff: number }[];
  matchKindDiffs: { name: string; tsKind: string; cppKind: string }[];
  orderingDiffs: { position: number; tsName: string; cppName: string }[];
  tsOnlyNames: string[];
  cppOnlyNames: string[];
}

const tsResults: Record<string, ResultEntry[]> = JSON.parse(
  fs.readFileSync(__dirname + "/ts_results.json", "utf-8")
);
const cppResults: Record<string, ResultEntry[]> = JSON.parse(
  fs.readFileSync(__dirname + "/cpp_results.json", "utf-8")
);

const SCORE_EPSILON = 0.01;
const reports: ComparisonReport[] = [];
let totalQueries = 0;
let exactMatchQueries = 0;
let scoreDiffCount = 0;
let matchKindDiffCount = 0;
let orderingDiffCount = 0;
let nameSetDiffCount = 0;

for (const query of Object.keys(tsResults)) {
  totalQueries++;
  const tsR = tsResults[query] || [];
  const cppR = cppResults[query] || [];

  const tsNames = new Set(tsR.map((r) => r.name));
  const cppNames = new Set(cppR.map((r) => r.name));

  const tsOnly = tsR.filter((r) => !cppNames.has(r.name)).map((r) => r.name);
  const cppOnly = cppR.filter((r) => !tsNames.has(r.name)).map((r) => r.name);

  const namesMatch =
    tsR.length === cppR.length &&
    tsOnly.length === 0 &&
    cppOnly.length === 0;

  if (namesMatch) nameSetDiffCount++;
  if (namesMatch && tsOnly.length === 0 && cppOnly.length === 0) {
    // Check ordering
    let orderOk = true;
    for (let i = 0; i < tsR.length; i++) {
      if (tsR[i].name !== cppR[i].name) {
        orderOk = false;
        break;
      }
    }
    if (orderOk) exactMatchQueries++;
  }

  const scoreDiffs: ComparisonReport["scoreDiffs"] = [];
  const matchKindDiffs: ComparisonReport["matchKindDiffs"] = [];
  const orderingDiffs: ComparisonReport["orderingDiffs"] = [];

  // Compare common entries
  for (let i = 0; i < Math.min(tsR.length, cppR.length); i++) {
    const ts = tsR[i];
    const cpp = cppR[i];
    const diff = Math.abs(ts.score - cpp.score);
    if (diff > SCORE_EPSILON) {
      scoreDiffs.push({
        name: ts.name,
        tsScore: ts.score,
        cppScore: cpp.score,
        diff: parseFloat(diff.toFixed(4)),
      });
      scoreDiffCount++;
    }
    if (ts.matchKind !== cpp.matchKind) {
      matchKindDiffs.push({
        name: ts.name,
        tsKind: ts.matchKind,
        cppKind: cpp.matchKind,
      });
      matchKindDiffCount++;
    }
    if (ts.name !== cpp.name) {
      orderingDiffs.push({
        position: i,
        tsName: ts.name,
        cppName: cpp.name,
      });
      orderingDiffCount++;
    }
  }

  reports.push({
    query,
    tsCount: tsR.length,
    cppCount: cppR.length,
    namesMatch,
    scoreDiffs,
    matchKindDiffs,
    orderingDiffs,
    tsOnlyNames: tsOnly,
    cppOnlyNames: cppOnly,
  });
}

// Summary
console.log("\n=== PARITY REPORT ===\n");
console.log(`Queries tested: ${totalQueries}`);
console.log(
  `Exact match (names + ordering + scores + kinds): ${exactMatchQueries}/${totalQueries}`
);
console.log(`Name set differences: ${scoreDiffCount} score diffs`);
console.log(`MatchKind differences: ${matchKindDiffCount}`);
console.log(`Ordering differences: ${orderingDiffCount}`);
console.log("");

// Detailed per-query report
for (const r of reports) {
  const hasIssues =
    !r.namesMatch ||
    r.scoreDiffs.length > 0 ||
    r.matchKindDiffs.length > 0 ||
    r.orderingDiffs.length > 0;

  if (hasIssues) {
    console.log(`--- ${r.query} ---`);
    console.log(`  TS results: ${r.tsCount}, C++ results: ${r.cppCount}`);
    if (r.tsOnlyNames.length > 0) {
      console.log(`  TS-only: ${r.tsOnlyNames.join(", ")}`);
    }
    if (r.cppOnlyNames.length > 0) {
      console.log(`  C++-only: ${r.cppOnlyNames.join(", ")}`);
    }
    for (const d of r.scoreDiffs) {
      console.log(
        `  Score diff: ${d.name} — TS=${d.tsScore} C++=${d.cppScore} (Δ=${d.diff})`
      );
    }
    for (const d of r.matchKindDiffs) {
      console.log(
        `  Kind diff: ${d.name} — TS=${d.tsKind} C++=${d.cppKind}`
      );
    }
    for (const d of r.orderingDiffs) {
      console.log(
        `  Order diff at #${d.position + 1}: TS="${d.tsName}" vs C++="${d.cppName}"`
      );
    }
    console.log("");
  }
}

// Write report
fs.writeFileSync(
  __dirname + "/parity_report.json",
  JSON.stringify({ summary: { totalQueries, exactMatchQueries, scoreDiffCount, matchKindDiffCount, orderingDiffCount }, queries: reports }, null, 2)
);
console.log("Parity report written to parity_report.json");
