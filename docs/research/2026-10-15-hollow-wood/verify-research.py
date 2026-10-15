"""Validate this research handoff's structure; no animation claims."""
import csv
import hashlib
import json
from decimal import Decimal
from pathlib import Path
import re
import xml.etree.ElementTree as ET
from PIL import Image

root = Path(__file__).resolve().parent
rows = list(csv.DictReader((root / '20-candidate-scores.csv').open(encoding='utf-8-sig', newline='')))
assert len(rows) == 20 and {r['id'] for r in rows} == {f'{i:02}' for i in range(1, 21)}
selected = [r['id'] for r in rows if r['portfolio_selected'] == 'yes']
assert selected == ['01', '02', '04', '17', '19']
criteria = [('forest_spectral_fit_20pct', '.20'), ('distinctive_idea_20pct', '.20'),
            ('motion_choreography_20pct', '.20'), ('projected_readability_15pct', '.15'),
            ('production_feasibility_15pct', '.15'), ('evidence_quality_10pct', '.10')]
for row in rows:
    total = sum(Decimal(row[key]) * Decimal(weight) for key, weight in criteria)
    assert total == Decimal(row['weighted_concept_screen_estimate']), row['id']
roster = (root / 'AGENT-ROSTER.md').read_text(encoding='utf-8')
tasks = set(re.findall(r'/root/[a-z_]+', roster))
assert len(tasks) == 15
reports = sorted(p.name for p in root.glob('*.md') if re.match(r'(?:0[1-9]|1[0-5])-', p.name))
assert len(reports) == 15
ET.parse(root / '15-value-storyboards.svg')
images = {}
for path in root.glob('*.png'):
    with Image.open(path) as im:
        images[path.name] = list(im.size)
        im.verify()
assert len(images) == 4
bad_links = []
for path in root.glob('*.md'):
    for target in re.findall(r'\]\(([^)]+)\)', path.read_text(encoding='utf-8')):
        target = target.strip('<>').split('#', 1)[0]
        if not target or re.match(r'https?://|mailto:', target):
            continue
        if not (path.parent / target).exists():
            bad_links.append([path.name, target])
assert not bad_links, bad_links
links = list(csv.DictReader((root / 'source-links.csv').open(encoding='utf-8-sig', newline='')))
assert any(r['Url'] == 'https://pmc.ncbi.nlm.nih.gov/articles/PMC3324571/' and r['Report'] == '11-source-audit.md' for r in links)
result = {'scope': 'Static research artifacts and arithmetic only', 'candidate_count': 20,
          'selected_ids': selected, 'score_arithmetic': 'exact', 'roster_task_identities': len(tasks),
          'specialist_reports': len(reports), 'svg_xml': 'parsed', 'png_dimensions': images,
          'broken_local_markdown_links': bad_links, 'working_primary_alternate_in_manifest': True,
          'animation_tested': False, 'mapper_new_media_tested': False, 'projector_tested': False}
(root / 'verification.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
print(json.dumps(result, indent=2))
