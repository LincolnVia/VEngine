return {
	{
		"saghen/blink.cmp",
		version = "1.*",
		event = "InsertEnter",
		dependencies = { "rafamadriz/friendly-snippets" },
		opts = {
			keymap = {
				preset = "default",
				["<CR>"] = { "accept", "fallback" },
				["<C-j>"] = { "select_next", "fallback" },
				["<C-k>"] = { "select_prev", "fallback" },
				["<S-Tab>"] = { "snippet_backward", "show_signature", "fallback" },
			},
			completion = { documentation = { auto_show = true, auto_show_delay_ms = 300 } },
			signature = { enabled = true },
			sources = { default = { "lsp", "path", "snippets", "buffer" } },
			fuzzy = { implementation = "prefer_rust_with_warning" },
		},
	},
	{
		"mason-org/mason.nvim",
		cmd = "Mason",
		opts = {},
	},
	{
		"mason-org/mason-lspconfig.nvim",
		dependencies = {
			"mason-org/mason.nvim",
			"neovim/nvim-lspconfig",
			"saghen/blink.cmp",
		},
		opts = {
			ensure_installed = {
				"clangd",
				"rust_analyzer",
				"ts_ls",
				"eslint",
				"html",
				"cssls",
				"tailwindcss",
				"lua_ls",
			},
			automatic_enable = {
				"clangd", "rust_analyzer", "ts_ls", "eslint", "html", "cssls", "tailwindcss", "lua_ls",
			},
		},
		config = function(_, opts)
			require("mason").setup()

			local capabilities = require("blink.cmp").get_lsp_capabilities()
			local servers = { "clangd", "rust_analyzer", "ts_ls", "eslint", "html", "cssls", "tailwindcss", "lua_ls", "roslyn_ls" }

			for _, server in ipairs(servers) do
				vim.lsp.config(server, { capabilities = capabilities })
			end
			vim.lsp.config("roslyn_ls", {
				cmd = { "roslyn-language-server", "--stdio" },
				on_init = function(client)
					local root = client.config.root_dir
					if type(root) ~= "string" then
						return
					end
					local projects = vim.fn.globpath(root, "*.csproj", false, true)
					if #projects > 0 then
						client:notify("project/open", {
							projects = vim.tbl_map(vim.uri_from_fname, projects),
						})
					end
				end,
			})

			vim.lsp.config("clangd", {
				cmd = {
					"clangd",
					"--background-index",
					"--clang-tidy",
					"--completion-style=detailed",
					"--query-driver=/opt/homebrew/bin/g++-16",
				},
			})
			vim.lsp.config("rust_analyzer", {
				settings = {
					["rust-analyzer"] = {
						check = { command = "clippy" },
						cargo = { allFeatures = true },
					},
				},
			})
			vim.lsp.config("lua_ls", {
				settings = {
					Lua = {
						diagnostics = { globals = { "vim" } },
						workspace = { library = { vim.env.VIMRUNTIME } },
					},
				},
			})

			require("mason-lspconfig").setup(opts)

			local function start_roslyn(buf)
				if #vim.lsp.get_clients({ bufnr = buf, name = "roslyn_ls" }) > 0 then
					return
				end

				local config = vim.deepcopy(vim.lsp.config.roslyn_ls)
				local root_dir
				config.root_dir(buf, function(root)
					root_dir = root
				end)
				if root_dir then
					config.root_dir = root_dir
					vim.lsp.start(config, { bufnr = buf })
				end
			end

			-- Roslyn is attached explicitly so it is the only C# language server.
			vim.api.nvim_create_autocmd("FileType", {
				group = vim.api.nvim_create_augroup("volt-roslyn", { clear = true }),
				pattern = "cs",
				callback = function(event)
					start_roslyn(event.buf)
				end,
			})
			for _, buf in ipairs(vim.api.nvim_list_bufs()) do
				if vim.api.nvim_buf_is_loaded(buf)
					and vim.bo[buf].filetype == "cs"
					and #vim.lsp.get_clients({ bufnr = buf, name = "roslyn_ls" }) == 0
				then
					start_roslyn(buf)
				end
			end

			vim.diagnostic.config({
				severity_sort = true,
				float = { border = "rounded", source = true },
				underline = true,
				signs = true,
				virtual_text = false,
				virtual_lines = { current_line = true },
			})

			vim.api.nvim_create_autocmd("LspAttach", {
				group = vim.api.nvim_create_augroup("volt-lsp-attach", { clear = true }),
				callback = function(event)
					local map = function(lhs, rhs, desc)
						vim.keymap.set("n", lhs, rhs, { buffer = event.buf, desc = desc })
					end
					local builtin = require("telescope.builtin")

					local function pick_lsp_location(picker, title)
						picker({
							prompt_title = title,
							attach_mappings = function(prompt_bufnr, map_picker)
								local actions = require("telescope.actions")
								local action_state = require("telescope.actions.state")
								local function choose_window()
									local entry = action_state.get_selected_entry()
									if not entry then
										return
									end
									actions.close(prompt_bufnr)
									vim.ui.select({ "Current pane", "Vertical split", "Horizontal split" }, {
										prompt = "Open " .. title:lower() .. " in:",
									}, function(choice)
										if not choice then
											return
										end
										if choice == "Vertical split" then
											vim.cmd("vsplit")
										elseif choice == "Horizontal split" then
											vim.cmd("split")
										end
										vim.lsp.util.jump_to_location(entry.value, "utf-16", true)
									end)
								end
								map_picker("i", "<CR>", choose_window)
								map_picker("n", "<CR>", choose_window)
								return true
							end,
						})
					end

					map("gd", function()
						pick_lsp_location(builtin.lsp_definitions, "Definitions")
					end, "Go to definition")
					local function go_to_implementation()
						local bufnr = event.buf
						local method = "textDocument/implementation"
						if #vim.lsp.get_clients({ bufnr = bufnr, method = method }) == 0 then
							pick_lsp_location(builtin.lsp_definitions, "Definitions")
							return
						end

						local win = vim.api.nvim_get_current_win()
						vim.lsp.buf_request_all(bufnr, method, function(client)
							return vim.lsp.util.make_position_params(win, client.offset_encoding)
						end, function(responses)
							if vim.api.nvim_get_current_buf() ~= bufnr then
								return
							end
							for _, response in pairs(responses) do
								local locations = response.result
								if type(locations) == "table" and next(locations) ~= nil then
									pick_lsp_location(builtin.lsp_implementations, "Implementations")
									return
								end
							end
							pick_lsp_location(builtin.lsp_definitions, "Definitions")
						end)
					end
					map("<leader>lo", go_to_implementation, "Go to implementation or definition")
					map("gr", builtin.lsp_references, "Go to references")
					map("gI", function()
						pick_lsp_location(builtin.lsp_implementations, "Implementations")
					end, "Go to implementation")
					map("<leader>go", go_to_implementation, "Go to implementation")
					map("<leader>ls", builtin.lsp_document_symbols, "Document symbols")
					map("<leader>le", function()
						builtin.diagnostics({ bufnr = nil })
					end, "Workspace diagnostics")
					map("<leader>lS", builtin.lsp_dynamic_workspace_symbols, "Workspace symbols")
					map("<leader>lr", vim.lsp.buf.rename, "Rename symbol")
					map("<leader>la", vim.lsp.buf.code_action, "Code action")
					map("<leader>ld", vim.diagnostic.open_float, "Line diagnostics")
					map("<leader>lh", function()
						vim.lsp.inlay_hint.enable(
							not vim.lsp.inlay_hint.is_enabled({ bufnr = event.buf }),
							{ bufnr = event.buf }
						)
					end, "Toggle inlay hints")
				end,
			})
		end,
	},
}
