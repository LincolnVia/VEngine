return {
  {
    "shadowchiki/nvim-cpp-tools",
    dependencies = { "nvim-treesitter/nvim-treesitter" },
    config = function()
      require("nvim-cpp-tools").setup()
    end,
  },
}
