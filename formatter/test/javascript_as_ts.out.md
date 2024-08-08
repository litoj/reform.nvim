```ts
config(___...configs: ConfigWithExtends[]___): FlatConfig.ConfigArray```

Utility function to make it easy to strictly type your "Flat" config file

**Example**:
```ts
// _@ts-check_ —
import eslint from '@eslint/js';
import tseslint from 'typescript-eslint';

export default tseslint.config(
  eslint.configs.recommended,
  ...tseslint.configs.recommended,
  {
    rules: {
      '@typescript-eslint/array-type': 'error',
    },
  },
);```
