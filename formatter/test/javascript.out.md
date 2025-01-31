```ts
/*property*/ config: (...configs: ConfigWithExtends[]) => FlatConfig.ConfigArray```

Utility function to make it easy to strictly type your "Flat" config file

**Example**:
```js
//
**Ts-check**: import eslint from '@eslint/js';
import tseslint from 'typescript-eslint';
export default tseslint.config(
  eslint.configs.recommended,
  ...tseslint.configs.recommended,
  {
    rules: {
      '@typescript-eslint/array-type': 'error',
    },
  },
);
```
