// Prints the CHANGELOG.md section for the given version (used by the release workflow).
import { readFileSync } from "node:fs";

const version = process.argv[2];
const text = readFileSync(new URL("../CHANGELOG.md", import.meta.url), "utf8");
const start = text.indexOf(`## [${version}]`);
if (start === -1) {
  console.log(`Release ${version}`);
  process.exit(0);
}
const rest = text.slice(start).split("\n").slice(1);
const end = rest.findIndex((line) => line.startsWith("## "));
console.log((end === -1 ? rest : rest.slice(0, end)).join("\n").trim());
