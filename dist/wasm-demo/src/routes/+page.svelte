<script lang="ts">
  import { PaneGroup, Pane, PaneResizer } from "paneforge";
  import { Terminal } from "@xterm/xterm";
  import { FitAddon } from "@xterm/addon-fit";
  import { onMount } from "svelte";
  import * as monaco from "monaco-editor";

  import "@xterm/xterm/css/xterm.css";
  import {
    WASI,
    File,
    OpenFile,
    ConsoleStdout,
    PreopenDirectory,
  } from "@bjorn3/browser_wasi_shim";

  const example_basic_entry = (str, title) => {
    return {
      value: str,
      title: title ?? `s:${str}`,
      template: `./examples/basics/${str}/templ.xml`,
      data: `./examples/basics/${str}/data.xml`,
    };
  };

  const examples = [
    {
      title: "Basics",
      items: [
        example_basic_entry("default", "Hello world!"),
        example_basic_entry("value"),
        example_basic_entry("element"),
        example_basic_entry("for-range"),
        example_basic_entry("for"),
        example_basic_entry("for-props"),
        example_basic_entry("test"),
        example_basic_entry("data"),
        example_basic_entry("when"),
        //example_basic_entry("include"),
      ],
    },
    {
      title: "Expressions",
      items: [],
    },
    {
      title: "Demos",
      items: [
        {
          value: "HTML Generation",
          template: "./examples/advanced/html/templ.xml",
          data: "./examples/advanced/html/data.0.xml",
        },
        {
          value: "Tree-rewrite",
          template: "./examples/advanced/tree-rewrite/templ.xml",
          data: "./examples/advanced/tree-rewrite/data.xml",
        },
      ],
    },
  ];

  let editorRef: HTMLDivElement;
  let readerRef: HTMLDivElement;
  let terminalRef: HTMLDivElement;
  let genButtonRef: HTMLButtonElement;

  let editor: any;
  let reader: any;

  let current_view: "template" | "data" | "extra" | null = $state(null);
  let example = $state("default");

  const change_view = (view: "template" | "data" | "extra") => {
    if (current_view == "template") {
      code_templ = editor.getValue();
    } else if (current_view == "data") {
      code_data = editor.getValue();
    } else if (current_view == "extra") {
      code_extra = editor.getValue();
    }
    current_view = view;
    if (current_view == "template") {
      editor.setValue(code_templ);
    } else if (current_view == "data") {
      editor.setValue(code_data);
    } else if (current_view == "extra") {
      editor.setValue(code_extra);
    }
  };

  const apply_example = async () => {
    //TODO: check in the examples and apply to code_templ and code_data
    for (const g of examples) {
      for (const h of g.items) {
        if (example == h.value) {
          code_data = await (await fetch(h.data)).text();
          code_templ = await (await fetch(h.template)).text();
        }
      }
    }
    if (current_view == "template") {
      editor.setValue(code_templ);
    } else if (current_view == "data") {
      editor.setValue(code_data);
    } else if (current_view == "extra") {
      editor.setValue(code_extra);
    }
  };

  let code_data = `<hello></hello>`;
  let code_templ = `<hello2></hello2>`;
  let code_extra = `<!-- Additional file, if you want to test extra data sources or include ->`;
  let generated_xml = `<!-- Click generate to see the resulting code ->`;

  let iframeRef: HTMLIFrameElement;

  const term = new Terminal();
  const fitAddon = new FitAddon();

  let module: WebAssembly.Module;

  onMount(async () => {
    editor = monaco.editor.create(editorRef, {
      value: code_data,
      language: "xml",
      automaticLayout: true,
      theme: "vs-dark",
    });

    reader = monaco.editor.create(readerRef, {
      value: generated_xml,
      language: "xml",
      automaticLayout: true,
      readOnly: true,
    });

    await apply_example();
    change_view("template");

    term.loadAddon(fitAddon);
    term.open(terminalRef);
    fitAddon.fit();

    const data = new Uint8Array(
      await (await fetch("vs.templ.js")).arrayBuffer()
    );

    module = new WebAssembly.Module(data);
    genButtonRef.disabled = false;
  });

  $effect(() => {
    return () => {
      editor && editor.dispose();
    };
  });

  const compute = async () => {
    generated_xml = "";
    let args = ["bin", "template.xml", "data.xml"];
    let env: string[] = [
      /*"FOO=bar"*/
    ];
    let fds = [
      new OpenFile(new File([])), // stdin
      ConsoleStdout.lineBuffered((msg: string) => {
        generated_xml += msg + "\n";
      }),
      ConsoleStdout.lineBuffered((msg) => term.write(`${msg}\n\r`)),
      new PreopenDirectory(
        ".",
        new Map([
          ["data.xml", new File(new TextEncoder().encode(code_data))],
          ["template.xml", new File(new TextEncoder().encode(code_templ))],
          ["extra.xml", new File(new TextEncoder().encode(code_extra))],
        ])
      ),
    ];
    let wasi = new WASI(args, env, fds);

    const instance = new WebAssembly.Instance(module, {
      wasi_snapshot_preview1: wasi.wasiImport,
    });

    wasi.start(instance);

    console.log(reader);
    reader.setValue(generated_xml);
  };

  //TODO: On page resize terminal fit.
</script>

<PaneGroup direction="horizontal" style="height:100%;">
  <Pane
    defaultSize={50}
    onResize={() => {
      fitAddon.fit();
    }}
  >
    <PaneGroup direction="vertical">
      <Pane defaultSize={50}>
        <nav style="padding-left:0px;">
          <div class="logo">
            <a href="https://lazy-eggplant.github.io/vs.templ">vs.templ</a>
          </div>
          <div class="tabs">
            <a
              onclick={() => change_view("template")}
              class:active={current_view == "template"}
            >
              <h1>Template</h1>
            </a>
            <a
              onclick={() => change_view("data")}
              class:active={current_view == "data"}
            >
              <h1>Data</h1>
            </a>
            <a
              onclick={() => change_view("extra")}
              class:active={current_view == "extra"}
            >
              <h1>Extra</h1>
            </a>
          </div>
          <div style="flex-grow: 1;"></div>
          <div>
            <select
              bind:value={example}
              onchange={async () => {
                genButtonRef.disabled = true;
                await apply_example();
                genButtonRef.disabled = false;
              }}
            >
              {#each examples as group}
                <optgroup label={group.title}>
                  {#each group.items as example}
                    <option value={example.value}
                      >{example.title ?? example.value}</option
                    >
                  {/each}
                </optgroup>
              {/each}
            </select>
          </div>
          <button
            bind:this={genButtonRef}
            onclick={async () => {
              genButtonRef.disabled = true;
              change_view(current_view as "template" | "data" | "extra");
              term.reset();
              await compute();
              if (generated_xml.indexOf("<html") != -1)
                iframeRef.src =
                  "data:text/html;charset=utf-8," + encodeURI(generated_xml);
              else
                iframeRef.src =
                  "data:text/xml;charset=utf-8," + encodeURI(generated_xml);
              genButtonRef.disabled = false;
            }}
            disabled>Generate</button
          >
        </nav>

        <div
          class="editor"
          bind:this={editorRef}
          style="height:100%;width:100%;"
        ></div>
      </Pane>
      <PaneResizer class="handle_v" />
      <Pane defaultSize={50} onResize={() => fitAddon.fit()}>
        <div style="height:100%;width:100%;background-color:black;">
          <div bind:this={terminalRef} style="height:100%;width:100%;"></div>
        </div>
      </Pane>
    </PaneGroup>
  </Pane>
  <PaneResizer class="handle_h"></PaneResizer>
  <Pane defaultSize={50}>
    <PaneGroup direction="vertical">
      <Pane defaultSize={50}>
        <nav>
          <h1 style="padding-left:1rem;">Result</h1>
        </nav>
        <div
          class="editor"
          bind:this={readerRef}
          style="height:100%;width:100%;"
        ></div>
      </Pane>
      <PaneResizer class="handle_v" />
      <Pane defaultSize={50}>
        <iframe
          title="rendering"
          style="height:100%;width:100%;"
          bind:this={iframeRef}
          src="empty-frame.html"
        ></iframe>
      </Pane>
    </PaneGroup>
  </Pane>
</PaneGroup>

<style>
  .editor {
    /*border-top: 10px solid #1e1e1e;*/
  }

  :global(.handle_h) {
    width: 10px;
    background-color: #484848;
  }
  :global(.handle_v) {
    height: 10px;
    background-color: #484848;
  }

  .active {
    color: white;
  }

  nav {
    background-color: black;
    color: white;
    height: 50px;
    display: flex;
    flex-direction: row;
    align-items: center;

    & > .tabs {
      display: flex;
      flex-direction: row;
      gap: 1rem;
      padding-left: 1rem;
    }

    select {
      height: 50px;
      line-height: 100%;
      border: 0px;
      background-color: rgb(27, 27, 27);
      color: white;
      padding-left: 0.5rem;
      padding-right: 0.5rem;
      display: block;

      &:hover {
        background-color: rgb(46, 46, 46);
      }
    }

    button {
      height: 100%;
      line-height: 100%;
      border: 0px;
      background-color: rgb(18, 88, 24);
      color: white;
      padding-left: 0.5rem;
      padding-right: 0.5rem;
      display: block;

      &:hover {
        background-color: rgb(31, 105, 24);
      }

      &:disabled {
        background-color: rgb(81, 105, 79);
        cursor: wait;
      }
    }

    a {
      cursor: pointer;
      color: gray;
    }

    & > .logo {
      background-color: rgb(72, 18, 88);
      height: 100%;
      line-height: 100%;
      align-content: center;

      a {
        color: white;
        text-decoration: none;
        font-weight: bold;
        height: 100%;
        padding: 0.5rem;
      }

      &:hover {
        background-color: rgb(86, 24, 105);
      }
    }
  }
</style>
