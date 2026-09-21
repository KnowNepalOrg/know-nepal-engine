// Self-contained TypeScript scoring script.
// Copies the exact logic from fuzzy.ts and rank.ts to avoid import issues.
// This is a READ-ONLY faithful reproduction of the TypeScript reference.

import * as fs from "fs";
import * as path from "path";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// --- fuzzy.ts ---

function normalize(value: string): string {
  return value
    .toLowerCase()
    .replace(/[^a-z0-9\u0900-\u097F]+/g, " ")
    .trim()
    .replace(/\s+/g, " ");
}

function tokenize(value: string): string[] {
  return normalize(value).split(" ").filter(Boolean);
}

function levenshtein(a: string, b: string): number {
  if (a === b) return 0;
  if (!a.length) return b.length;
  if (!b.length) return a.length;

  const matrix: number[][] = Array.from({ length: a.length + 1 }, () =>
    Array(b.length + 1).fill(0)
  );

  for (let i = 0; i <= a.length; i++) matrix[i][0] = i;
  for (let j = 0; j <= b.length; j++) matrix[0][j] = j;

  for (let i = 1; i <= a.length; i++) {
    for (let j = 1; j <= b.length; j++) {
      const cost = a[i - 1] === b[j - 1] ? 0 : 1;
      matrix[i][j] = Math.min(
        matrix[i - 1][j] + 1,
        matrix[i][j - 1] + 1,
        matrix[i - 1][j - 1] + cost
      );
    }
  }

  return matrix[a.length][b.length];
}

function similarity(a: string, b: string): number {
  const maxLen = Math.max(a.length, b.length);
  if (maxLen === 0) return 1;
  return 1 - levenshtein(a, b) / maxLen;
}

// --- rank.ts ---

const MIN_SIMILARITY = 0.72;

interface SearchEntry {
  id: string;
  name: string;
  nameNepali?: string;
  module: string;
  type: string;
  href: string;
  aliases?: string[];
  keywords?: string[];
  location?: string;
  meta?: Record<string, any>;
  popularity?: number;
}

interface ScoreDetail {
  kind: string;
  score: number;
}

function scoreEntry(entry: SearchEntry, query: string): ScoreDetail | null {
  const q = normalize(query);
  if (!q) return null;

  const name = normalize(entry.name);
  const nameNepali = entry.nameNepali ? normalize(entry.nameNepali) : "";
  const qTokens = tokenize(q);

  if (name === q) return { kind: "exact", score: 100 };
  if (nameNepali && nameNepali === q) return { kind: "exact", score: 99 };

  for (const alias of entry.aliases ?? []) {
    if (normalize(alias) === q) return { kind: "alias", score: 97 };
  }

  if (name.startsWith(q)) return { kind: "substring", score: 92 };
  if (name.includes(q)) return { kind: "substring", score: 86 };

  for (const alias of entry.aliases ?? []) {
    const a = normalize(alias);
    if (a.startsWith(q)) return { kind: "alias", score: 88 };
    if (a.includes(q)) return { kind: "alias", score: 80 };
  }

  const nameTokens = tokenize(entry.name);
  if (
    qTokens.length > 0 &&
    qTokens.every((qt) =>
      nameTokens.some((nt) => nt.startsWith(qt) || qt.startsWith(nt))
    )
  ) {
    return { kind: "fuzzy", score: 82 - qTokens.length * 3 };
  }

  const sim = similarity(name, q);
  if (sim >= MIN_SIMILARITY) {
    return { kind: "fuzzy", score: 60 + sim * 30 };
  }

  if (nameNepali) {
    const neSim = similarity(nameNepali, q);
    if (neSim >= MIN_SIMILARITY) {
      return { kind: "fuzzy", score: 55 + neSim * 30 };
    }
  }

  for (const keyword of entry.keywords ?? []) {
    const kw = normalize(keyword);
    if (q.includes(kw) || kw.includes(q)) {
      return { kind: "keyword", score: 40 };
    }
  }

  return null;
}

interface SearchResult {
  entry: SearchEntry;
  score: number;
  matchKind: string;
}

function searchStaticIndex(
  entries: SearchEntry[],
  query: string,
  limit = 24
): SearchResult[] {
  const results: SearchResult[] = [];

  for (const entry of entries) {
    const detail = scoreEntry(entry, query);
    if (!detail) continue;

    let score = detail.score;
    if (entry.popularity) {
      score += (entry.popularity / 100) * 8;
    }

    results.push({ entry, score, matchKind: detail.kind });
  }

  results.sort(
    (a, b) => b.score - a.score || a.entry.name.localeCompare(b.entry.name)
  );
  return results.slice(0, limit);
}

// --- Benchmark ---

const datasetPath = __dirname + "/dataset.json";
const entries: SearchEntry[] = JSON.parse(fs.readFileSync(datasetPath, "utf-8"));

const queries = [
  "kathmandu",
  "dashain",
  "chitwan",
  "Kathmandu Durbar Square",
  "tiger",
  "nepal",
  "kath",
  "tu",
  "province 1",
  "swayambhu",
  "bhaktapur",
  "lumbini",
  "annapurna",
  "everest",
  "gurung",
  "nepali",
  "school",
  "hospital",
  "university",
  "festival",
];

const limit = 24;

// Run and output results
const allResults: Record<string, any[]> = {};
for (const query of queries) {
  const results = searchStaticIndex(entries, query, limit);
  allResults[query] = results.map((r) => ({
    name: r.entry.name,
    score: r.score,
    matchKind: r.matchKind,
    module: r.entry.module,
    type: r.entry.type,
  }));
}

const outPath = __dirname + "/ts_results.json";
fs.writeFileSync(outPath, JSON.stringify(allResults, null, 2));
console.log(`TypeScript: ${queries.length} queries over ${entries.length} entries -> ${outPath}`);

// Performance measurement (inside the same process, no I/O)
const WARMUP = 1000;
const ITERATIONS = 10000;

// Warmup
for (let i = 0; i < WARMUP; i++) {
  for (const query of queries) {
    searchStaticIndex(entries, query, limit);
  }
}

// Timed run
const latencies: number[] = [];
for (let iter = 0; iter < ITERATIONS; iter++) {
  const start = performance.now();
  for (const query of queries) {
    searchStaticIndex(entries, query, limit);
  }
  const end = performance.now();
  latencies.push(end - start);
}

latencies.sort((a, b) => a - b);
const sum = latencies.reduce((a, b) => a + b, 0);
const avg = sum / latencies.length;
const p50 = latencies[Math.floor(latencies.length * 0.5)];
const p95 = latencies[Math.floor(latencies.length * 0.95)];
const p99 = latencies[Math.floor(latencies.length * 0.99)];

const timing = {
  engine: "typescript",
  datasetSize: entries.length,
  queryCount: queries.length,
  iterations: ITERATIONS,
  warmup: WARMUP,
  avgMs: parseFloat(avg.toFixed(4)),
  p50Ms: parseFloat(p50.toFixed(4)),
  p95Ms: parseFloat(p95.toFixed(4)),
  p99Ms: parseFloat(p99.toFixed(4)),
  totalMs: parseFloat(sum.toFixed(4)),
};

fs.writeFileSync(__dirname + "/ts_timing.json", JSON.stringify(timing, null, 2));
console.log("TypeScript timing:", JSON.stringify(timing, null, 2));
