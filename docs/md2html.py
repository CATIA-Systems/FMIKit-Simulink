import os, markdown2
import shutil

docs = os.path.dirname(os.path.abspath(__file__))
repo = os.path.dirname(docs)

# copy images
shutil.rmtree(os.path.join(repo, 'html', 'images'), ignore_errors=True)
shutil.copytree(os.path.join(repo, 'docs', 'images'), os.path.join(repo, 'html', 'images'))

# generate HTML files
template = open(os.path.join(docs, 'template.html'), 'r').read()

for filename, title in [('fmu_export', 'FMU Export'),
                        ('fmu_import', 'FMU Import'),
                        ('index',      'FMI Kit'   )]:

    with open(os.path.join(docs, filename + '.md')) as f:
        md = f.read()

    html = markdown2.markdown(md, extras=['tables', 'fenced-code-blocks'])
    output = template.replace('{{content}}', html)
    output = output.replace('{{title}}', title)

    with open(os.path.join(repo, 'html', filename + '.html'), 'w') as f:
        f.write(output)
